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
    if ("selectionStart" in control && "selectionEnd" in control) {
      var start = control.selectionStart;
      var end = control.selectionEnd;
      if (start < end) {
        var text = control.value;
        return {type:"value_selection", control:control, text:text.substring(start, end),
                text_prefix:text.substr(0, start), text_suffix:text.substr(end), start:start, end:end};
      }
    }
    return {type:"value", control:control, text:control.value};
  }

  return null;
}

function korektorSetText(data, textArray) {
  // Input and textarea fields
  if (data.type == "value" || data.type == "value_selection") {
    // Concatenate new text
    var text = '';
    for (var i in textArray)
      text += textArray[i][textArray[i].length > 1 ? 1 : 0];

    // Replace the text in the control
    if (data.type == "value") {
      data.control.value = text;
    } else /*if (data.type == "value_selection")*/ {
      data.control.value = data.text_prefix + text + data.text_suffix;
      data.control.selectionStart = data.start;
      data.control.selectionEnd = data.start + text.length;
    }
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
  control.disabled = true;
//  jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/api/suggestions',
//              {dataType: "json", data: {model: model, data: data.text, suggestions: edit ? 5 : 1}, type: "POST",
//    success: function(json) {
      var json = {result: []};
      var text = data.text;
      while (text)
        if (text.match(/^\s+/)) {
          var token = text.match(/^\s+/);
          text = text.replace(/^\s+/, "");
          json.result.push([token]);
        } else {
          var token = text.match(/^\S+/);
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
        control.disabled = false;
      }
//    },
//    error: function(jqXHR, textStatus) {
//      alert(chrome.i18n.getMessage("korektor_error"));
//      control.disabled = false;
//    }
//  });
}

function korektorEdit(data, textArray) {
  // Add dialog
  var style = 'z-index:123456789; color:#000; background-color:transparent; font-family:serif; font-size:16px;';
  jQuery('body').append(
    '<div style="'+style+'position:fixed; left:0px; right:0px; top:0px; bottom:0px" id="korektorEditDialog">\n' +
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
  function korektorEditDialogClose() {
    data.control.disabled = false;
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
    }, 500);
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
    html += '<br/><b>Custom</b><br/><input type="text" style="'+style+'background:#fff; border:1px solid #999; border-radius:4px; width: 100%" value="' + suggestion.text().replace('"', '&quot;') + '"/>';

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
