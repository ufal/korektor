// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

function korektorGetText(control) {
  // Input and textarea fields
  if ("value" in control) {
    var text = control.value;
    var selection = null;
    if ("selectionStart" in control && "selectionEnd" in control) {
      var start = control.selectionStart;
      var end = control.selectionEnd;
      if (start < end) selection = {start:start, end:end};
    }
    return {type:"value", control:control, text:text, selection:selection};
  }

  // Contenteditable fields
  if ("contentEditable" in control) {
    var text = '';
    var range = control.ownerDocument.getSelection();
    range = range != null && !range.isCollapsed && range.rangeCount == 1 ? range.getRangeAt(0) : null;
    var selection = {};

    function getText(node) {
      if ("contentDocument" in node) node = node.contentDocument.body;
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3) {
          if (range && range.startContainer == child) selection.start = text.length + range.startOffset;
          if (range && range.endContainer == child) selection.end = text.length + range.endOffset;
          text += child.textContent;
        } else {
          if (range && range.startContainer == node && range.startOffset == i) selection.start = text.length;
          if (range && range.endContainer == node && range.endOffset == i) selection.end = text.length;
          getText(child);
          if (child.nodeType == 1 && child.nodeName.search(/^br$/i) == 0) text += "\n";
          if (child.nodeType == 1 && child.nodeName.search(/^(p|div)$/i) == 0) text += "\n\n";
          if (range && range.startContainer == node && range.startOffset == i+1) selection.start = text.length;
          if (range && range.endContainer == node && range.endOffset == i+1) selection.end = text.length;
        }
      }
    }
    getText(control);
    return {type:"contentEditable", control:control, text:text, selection:("start" in selection && "end" in selection) ? selection : null};
  }

  return null;
}

function korektorSetText(gettext, data, textArray) {
  // Check that current value is still the same
  var new_data = korektorGetText(data.control);
  if (!new_data || new_data.text != data.text) {
      alert(gettext('korektor_text_modified_error'));
      return;
    }

  // Remove identical replacements from textArray
  for (var i in textArray)
    if (textArray[i].length > 1 && textArray[i][1] == textArray[i][0])
      textArray[i].splice(1, textArray[i].length - 1)

  // Input and textarea fields
  if (data.type == "value") {
    // Concatenate new text
    var text = '';
    for (var i in textArray)
      text += textArray[i][textArray[i].length > 1 ? 1 : 0];

    // Replace the text in the control and update selection if any
    if (data.selection) {
      data.control.value = data.text.substr(0, data.selection.start) + text + data.text.substr(data.selection.end);
      data.control.selectionStart = data.selection.start;
      data.control.selectionEnd = data.selection.start + text.length;
    } else
      data.control.value = text;
    data.control.focus();
  }

  // Contenteditable fields
  if (data.type == "contentEditable") {
    var index = 0;
    var range = data.selection ? document.createRange() : null;
    function setText(node) {
      if ("contentDocument" in node) node = node.contentDocument.body;
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3) {
          var prev_all = child.textContent, prev = prev_all, prev_prefix_len = -1, prev_suffix_index = -1;
          if (data.selection && data.selection.end-index >= 0 && data.selection.end-index <= prev_all.length) { prev_suffix_index = data.selection.end - index; prev = prev.substr(0, prev_suffix_index); }
          if (data.selection && data.selection.start-index >= 0 && data.selection.start-index <= prev_all.length) { prev_prefix_len = data.selection.start - index; prev = prev.substr(prev_prefix_len); }
          var prev_ori = prev, next = '';

          if (!data.selection || (data.selection.start < index + prev_all.length && data.selection.end > index)) while (prev) {
            while (textArray.length && !textArray[0][0]) textArray.shift();
            if (!textArray) break;
            var len = prev.length < textArray[0][0].length ? prev.length : textArray[0][0].length;
            if (textArray[0].length == 1) {
              next += prev.substr(0, len);
            } else if (textArray[0].length > 1 && textArray[0][0].length > len && textArray[0][1].substr(0, len) == prev.substr(0, len)) {
              next += textArray[0][1].substr(0, len);
              textArray[0][1] = textArray[0][1].substr(len);
            } else if (textArray[0].length > 1 && textArray[0][1]) {
              next += textArray[0][1];
              textArray[0][1] = "";
            }
            prev = prev.substr(len);
            textArray[0][0] = textArray[0][0].substr(len);
          }
          next += prev;

          if (prev_prefix_len >= 0) next = prev_all.substr(0, prev_prefix_len) + next;
          if (prev_suffix_index >= 0) next = next + prev_all.substr(prev_suffix_index);
          if (next != prev_all) child.textContent = next;
          index += prev_all.length;
          if (prev_prefix_len >= 0) range.setStart(child, prev_prefix_len);
          if (prev_suffix_index >= 0) range.setEnd(child, prev_suffix_index + next.length - prev_all.length);
        } else {
          if (data.selection && data.selection.start == index) range.setStart(node, i);
          if (data.selection && data.selection.end == index) range.setEnd(node, i);
          if (child.nodeType == 1 && child.nodeName.search(/^br$/i) == 0) {
            if (!data.selection || (data.selection.start < index + 1 && data.selection.end > index)) {
              while (textArray.length && !textArray[0][0]) textArray.shift();
              if (textArray.length && textArray[0][0][0] == "\n") textArray[0][0] = textArray[0][0].substr(1);
            }
            index++;
          } else setText(child);
          if (data.selection && data.selection.start == index) range.setStart(node, i+1);
          if (data.selection && data.selection.end == index) range.setEnd(node, i+1);
        }
      }
    }
    setText(data.control);
    if (data.selection) {
      var selection = window.getSelection();
      if (selection) {
        selection.removeAllRanges();
        selection.addRange(range);
      }
    }
    data.control.focus();
  }
}

function korektorReport(data, korektorArray, correctedArray) {
  var original = data.selection ? data.text.substring(data.selection.start, data.selection.end) : data.text;
  var korektor = '';
  for (var i in korektorArray)
    korektor += korektorArray[i][korektorArray[i].length > 1 ? 1 : 0];
  var corrected = '';
  for (var i in correctedArray)
    corrected += correctedArray[i][correctedArray[i].length > 1 ? 1 : 0];

  jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/log.php',
              {dataType: "json", data: {original: original, korektor:korektor, corrected:corrected, origin:"korektor_plugin"}, type: "POST"});
}

function korektorPerformSpellcheck(gettext, control, model, edit) {
  // Chrome does not give us the control, find it manually.  Note that
  // when the contenteditable is inside an iframe, document.activeElement
  // contains only the iframe, so we need to hop over it ourselves.
  if (!control) {
    control = document.activeElement;
    while (control && control.contentDocument && control.contentDocument.activeElement && control.contentDocument.activeElement != control)
      control = control.contentDocument.activeElement;
  }

  // Abort if no control was given
  if (!control) return;

  // Ignore if edit is in progress
  if (jQuery('#korektorEditDialog').length > 0) return;

  // Try getting original text
  var data = korektorGetText(control);
  if (!data) return;

  // Run Korektor
  jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/api/suggestions',
              {dataType: "json", data: {model: model, data: data.selection ? data.text.substring(data.selection.start, data.selection.end) : data.text, suggestions: edit ? 5 : 1}, type: "POST",
    success: function(json) {
      if (!("result" in json)) {
        alert(gettext('korektor_service_error'));
        return;
      }
      var textArray = [];
      for (var i in json.result)
        if (json.result[i].length)
          textArray.push(json.result[i]);

      // Edit result or use it directly
      if (edit) {
        korektorEdit(gettext, data, textArray);
      } else {
        korektorSetText(gettext, data, textArray);
      }
    },
    error: function(jqXHR, textStatus) {
      alert(gettext('korektor_service_error'));
    }
  });
}

function korektorEdit(gettext, data, textArray) {
  // Add dialog
  var style = 'z-index:123456789; color:#000; background-color:transparent; font-family:serif; font-size:16px;';
  jQuery('body').append(
    '<div style="'+style+'position:fixed; left:0px; right:0px; top:0px; bottom:0px" id="korektorEditDialog" tabindex="-1">\n' +
    ' <div style="'+style+'position:absolute; left:0px; right:0px; top:0px; bottom:0px; background-color:#000; opacity:0.5"></div>\n' +
    ' <div style="'+style+'position:absolute; left:20px; right:20px; top:20px; bottom:20px; min-height:80px; background-color:#eee; border:1px solid #999; border-radius:6px; box-shadow: 0px 5px 15px rgba(0,0,0,0.5);">\n' +
    '  <div style="'+style+'position:absolute; width:100%; top:0px; height:36px; line-height:36px; text-align:center">\n' +
    '   <span style="'+style+'font-weight:bold; font-size:20px">Korektor Spellchecker</span>\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; left:8px; right:8px; top:36px; bottom:44px; background-color:#fff; border-radius:4px; border:1px solid #999; overflow-x: hidden; overflow-y: visible; white-space: pre-wrap" id="korektorEditText">\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; width:100%; bottom:0px; height:44px;">\n' +
    '   <div style="'+style+'position:absolute; left:0px; width:50%; height:100%; line-height:44px; text-align:center">\n' +
    '    <input type="checkbox" id="korektorEditReportText" style="'+style+'cursor:pointer" checked>\n' +
    '    <label style="'+style+'cursor:pointer" id="korektorEditReportTextLabel">'+gettext('korektor_dlg_report_text')+'</label>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:50%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:8px; right:4px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditOk">'+gettext('korektor_dlg_ok')+'</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:75%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:4px; right:8px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditCancel">'+gettext('korektor_dlg_cancel')+'</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '  </div>\n' +
    ' </div>\n' +
    '</div>\n'
  );
  jQuery('#korektorEditDialog').focus();

  function korektorEditDialogClose() {
    jQuery('#korektorEditDialog').remove();
    jQuery(document).off('keyup', korektorEditDialogHandleEscape);
  }
  function korektorEditDialogHandleEscape(event) {
    var dialog = jQuery('#korektorEditDialog');
    if (dialog.length > 0 && event.keyCode == 27) korektorEditDialogClose();
    else if (!dialog.length) jQuery(document).off('keyup', korektorEditDialogHandleEscape);
  }
  jQuery(document).keyup(korektorEditDialogHandleEscape);

  // Fill suggestions
  var html = '';
  var quoter = jQuery('<div/>');
  for (var i in textArray)
    if (textArray[i].length == 1) {
      html += quoter.text(textArray[i][0]).html();
    } else {
      html += '<span style="'+style+'color:#800;' + (textArray[i].length > 2 ? 'text-decoration: underline;' : '') +
              '" id="korektorEditSuggestion' + i + '">' + quoter.text(textArray[i][1]).html() + '</span>';
    }
  html += '<div style="'+style+'height: 5em;"></div>';
  html += '<div style="'+style+'position:absolute; background-color:#eee; padding:10px; border:1px solid #999; border-radius:6px; box-shadow: 0px 5px 15px rgba(0,0,0,0.5); display:none" id="korektorEditSuggestions"></div>';
  jQuery('#korektorEditText').html(html);

  // Suggestion dialog handling
  var korektorEditSuggestionsHovering = false;
  var korektorEditSuggestionsHideTimeout = null;
  function korektorEditSuggestionsMouseEnter() {
    korektorEditSuggestionsHovering = true;
  }
  function korektorEditSuggestionsMouseLeave() {
    korektorEditSuggestionsHovering = false;
    if (korektorEditSuggestionsHideTimeout !== null) clearTimeout(korektorEditSuggestionsHideTimeout);
    korektorEditSuggestionsHideTimeout = setTimeout(function(){
      korektorEditSuggestionsHideTimeout = null;
      if (!korektorEditSuggestionsHovering) jQuery('#korektorEditSuggestions').hide()
    }, 300);
  }
  function korektorEditSuggestionsUpdate(suggestion) {
    var suggestion_text = suggestion.text();
    jQuery('#korektorEditSuggestions span').each(function() {
      var span = jQuery(this);
      span.css('font-weight', span.text() == suggestion_text ? 'bold' : 'normal');
    });
  }
  function korektorEditSuggestionsFill(suggestion) {
    var id = suggestion.attr('id').replace(/^korektorEditSuggestion/, '');

    var html = '<b>'+gettext('korektor_dlg_suggestions_original')+'</b>';
    var quoter = jQuery('<div/>');
    for (var i in textArray[id]) {
      if (i == 1) html += '<br/><b>'+gettext('korektor_dlg_suggestions')+'</b>';
      html += '<br><span style="'+style+'color:#800; text-decoration:underline; cursor:pointer;">' + quoter.text(textArray[id][i]).html() + '</span>';
    }
    html += '<br/><b>'+gettext('korektor_dlg_suggestions_custom')+'</b><br/><input type="text" style="'+style+'background:#fff; border:1px solid #999; border-radius:4px; width: 100%" value="' + suggestion.text().replace(/"/g, '&quot;') + '"/>';

    jQuery('#korektorEditSuggestions')
      .empty()
      .show()
      .offset({left: suggestion.offset().left, top: suggestion.offset().top + suggestion.height()})
      .html(html);
    korektorEditSuggestionsUpdate(suggestion);
    jQuery('#korektorEditSuggestions span').click(function() {
      suggestion.text(jQuery(this).text());
      korektorEditSuggestionsUpdate(suggestion);
    });
    jQuery('#korektorEditSuggestions input').on('change input',function() {
      suggestion.text(jQuery(this).val());
      korektorEditSuggestionsUpdate(suggestion);
    });
  }
  function korektorEditSuggestionsShow() {
    korektorEditSuggestionsMouseEnter();
    korektorEditSuggestionsFill(jQuery(this));
  }
  jQuery('#korektorEditSuggestions').hover(korektorEditSuggestionsMouseEnter, korektorEditSuggestionsMouseLeave);
  jQuery('#korektorEditText span').hover(korektorEditSuggestionsShow, korektorEditSuggestionsMouseLeave)

  // Dialog controls
  jQuery('#korektorEditReportTextLabel').click(function() {
    jQuery('#korektorEditReportText').prop('checked', !jQuery('#korektorEditReportText').prop('checked'));
  });
  jQuery('#korektorEditOk').click(function() {
    var resultArray = [];
    for (var i in textArray)
      resultArray.push(textArray[i].length == 1 ? textArray[i] : [textArray[i][0], jQuery('#korektorEditSuggestion'+i).text()]);
    if (jQuery('#korektorEditReportText').prop('checked')) korektorReport(data, textArray, resultArray);
    korektorSetText(gettext, data, resultArray);
    korektorEditDialogClose();
  });
  jQuery('#korektorEditCancel').click(korektorEditDialogClose);
}
