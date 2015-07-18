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
    var text_prefix = '', text_suffix = '';
    var selection_start = -1;
    if ("selectionStart" in control && "selectionEnd" in control) {
      var start = control.selectionStart;
      var end = control.selectionEnd;
      if (start < end) {
        selection_start = start;
        text_prefix = text.substr(0, start);
        text_suffix = text.substr(end);
        text = text.substring(start, end);
      }
    }
    return {type:"value", control:control, text:text, text_prefix:text_prefix, text_suffix:text_suffix, selection_start:selection_start};
  }

  // Contenteditable fields
  if ("contentEditable" in control) {
    var text = '';
    function getText(node) {
      if ("contentDocument" in node) node = node.contentDocument.body;
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3)
          text += child.textContent;
        else if (child.nodeType == 1 && child.nodeName.search(/^br$/i) == 0)
          text += "\n";
        else
          getText(child);
      }
    }
    getText(control);
    return {type:"contentEditable", control:control, text:text};
  }

  return null;
}

function korektorSetText(data, textArray) {
  // Check that current value is still the same
  var new_data = korektorGetText(data.control);
  if (new_data.text != data.text) {
      alert("The text of the editable fields has changed, not replacing it.");
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

    // Replace the text in the control
    data.control.value = data.text_prefix + text + data.text_suffix;

    // Update selection if there was any
    if (data.selection_start != -1) {
      data.control.selectionStart = data.text_prefix.length;
      data.control.selectionEnd = data.text_prefix.length + text.length;
    }
  }

  // Contenteditable fields
  if (data.type == "contentEditable") {
    function setText(node) {
      if ("contentDocument" in node) node = node.contentDocument.body;
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3) {
          var prev = child.textContent, prev_ori = prev, next = '';
          while (prev) {
            while (textArray && !textArray[0][0]) textArray.shift();
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
          if (next != prev_ori) child.textContent = next;
        } else if (child.nodeType == 1 && child.nodeName.search(/^br$/i) == 0) {
          while (textArray && !textArray[0][0]) textArray.shift();
          if (textArray && textArray[0][0][0] == "\n") textArray[0][0] = textArray[0][0].substr(1);
        } else {
          setText(child);
        }
      }
    }
    setText(data.control);
  }
}

function korektorPerformSpellcheck(model, edit) {
  // Ignore if edit is in progress
  if (jQuery('#korektorEditDialog').length > 0) return;

  // Get current control
  var control = document.activeElement;
  if (!control) return;

  // Try getting original text
  var data = korektorGetText(control);
  if (!data) return;

  // Run Korektor
//  jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/api/suggestions',
//              {dataType: "json", data: {model: model, data: data.text, suggestions: edit ? 5 : 1}, type: "POST",
//    success: function(json) {
      var json = {result: []};
      var text = data.text;
      while (text)
        if (text.match(/^\s+/)) {
          var token = text.match(/^\s+/)[0];
          text = text.replace(/^\s+/, "");
          json.result.push([token]);
        } else {
          var token = text.match(/^\S+/)[0];
          text = text.replace(/^\S+/, "");
          json.result.push([token, token]);
        }

      if (!("result" in json)) {
        alert(chrome.i18n.getMessage("korektor_error"));
        return;
      }
      var textArray = [];
      for (var i in json.result)
        if (json.result[i].length)
          textArray.push(json.result[i]);

      // Edit result or use it directly
      if (edit) {
        korektorEdit(data, textArray);
      } else {
        korektorSetText(data, textArray);
      }
//    },
//    error: function(jqXHR, textStatus) {
//      alert(chrome.i18n.getMessage("korektor_error"));
//    }
//  });
}

function korektorEdit(data, textArray) {
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
    '    <label style="'+style+'cursor:pointer" id="korektorEditReportTextLabel">Report corrected text</label>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:50%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:8px; right:4px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditOk">OK</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:75%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:4px; right:8px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditCancel">Cancel</button>\n' +
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

    var html = '<b>Original</b>';
    var quoter = jQuery('<div/>');
    for (var i in textArray[id]) {
      if (i == 1) html += '<br/><b>Suggestions</b>';
      html += '<br><span style="'+style+'color:#800; text-decoration:underline; cursor:pointer;">' + quoter.text(textArray[id][i]).html() + '</span>';
    }
    html += '<br/><b>Custom</b><br/><input type="text" style="'+style+'background:#fff; border:1px solid #999; border-radius:4px; width: 100%" value="' + suggestion.text().replace(/"/g, '&quot;') + '"/>';

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
    korektorSetText(data, resultArray);
    korektorEditDialogClose();
  });
  jQuery('#korektorEditCancel').click(korektorEditDialogClose);
}
