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
  if ("value" in control)
    return control.value;

  return null;
}

function korektorSetText(control, textArray) {
  // Input and textarea fields
  if ("value" in control) {
    var text = '';
    for (var i in textArray)
      if (textArray[i].length > 0)
        text += textArray[i][textArray[i].length > 1 ? 1 : 0];
    control.value = text;
  }
}

function korektorPerformSpellcheck(model, edit) {
  // Ignore if edit is in progress
  if (jQuery('#korektorEditDialog').length > 0) return;

  // Get current control
  var control = document.activeElement;
  if (!control) return;

  korektorEdit(control, [["ahoj "], ["idi", "lidi"], [", "], ["ak", "jak", "jestli"], [" se "], ["mate", "máte", "mateš"], ["?"], [" megadlouhéslovo", " megamegadlouhéslovo"]]);
  return;

  // Try getting original text
  var text = korektorGetText(control);
  if (!text) return;

  // Run Korektor
  control.disabled = true;
  jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/api/suggestions',
              {dataType: "json", data: {model: model, data: text, suggestions: edit ? 5 : 1}, type: "POST",
    success: function(json) {
      if (!("result" in json)) {
        alert(chrome.i18n.getMessage("korektor_error"));
        return;
      }

      // Edit result or use it directly
      if (edit)
        korektorEdit(control, json.result);
      else
        korektorSetText(control, json.result);
    },
    error: function(jqXHR, textStatus) {
      alert(chrome.i18n.getMessage("korektor_error"));
    },
    complete: function() {
      control.disabled = false;
    }
  });
}

function korektorEdit(control, textArray) {
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
    jQuery('#korektorEditDialog').remove();
    jQuery(document).off('keyup', korektorEditDialogHandleEscape);
  }
  function korektorEditDialogHandleEscape(event) {
    var dialog = jQuery('#korektorEditDialog');
    if (dialog.length > 0 && event.keyCode == 27) dialog.remove();
    if (!dialog.length || event.keyCode == 27) jQuery(document).off('keyup', korektorEditDialogHandleEscape);
  }
  jQuery(document).keyup(korektorEditDialogHandleEscape);

  // Fill suggestions
  var html = '';
  for (var i in textArray)
    if (textArray[i].length <= 1) {
      html += textArray[i].length == 1 ? textArray[i][0] : '';
    } else {
      html += '<span style="'+style+'color:#800;' + (textArray[i].length > 2 ? 'text-decoration: underline;' : '') +
              '" id="korektorEditSuggestion' + i + '">' + textArray[i][1] + '</span>';
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
    for (var i in textArray[id]) {
      if (i == 1) html += '<br/><b>Suggestions</b>';
      html += '<br><span style="'+style+'color:#800; text-decoration:underline; cursor:pointer;">' + textArray[id][i] + '</span>';
    }
    html += '<br/><b>Custom</b><br/><input type="text" style="'+style+'background:#fff; border:1px solid #999; border-radius:4px; width: 100%" value="' + suggestion.text() + '"/>';

    jQuery('#korektorEditSuggestions')
      .html('')
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
      resultArray.push(textArray[i].length <= 1 ? textArray[i] : [textArray[i][0], jQuery('#korektorEditSuggestion'+i).text()]);
    korektorSetText(control, resultArray);
    korektorEditDialogClose();
  });
  jQuery('#korektorEditCancel').click(korektorEditDialogClose);
}
