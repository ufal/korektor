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
      text += textArray[i][textArray[i].length > 1 ? 1 : 0];
    control.value = text;
  }
}

function korektorPerformSpellcheck(model, edit) {
  // Get current control
  var control = document.activeElement;
  if (!control) return;

  korektorEdit(control, []);
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

var korektorEditControl;
var korektorEditTextArray;

function korektorEdit(control, textArray) {
  korektorEditControl = control;
  korektorEditTextArray = textArray;

  var style = 'z-index:123456789; color:#000; background-color:transparent; font-family:serif; font-size:16px;';

  jQuery('body').append(
    '<div style="'+style+'position:fixed; left:0px; right:0px; top:0px; bottom:0px" id="korektorEditDialog">\n' +
    ' <div style="'+style+'position:absolute; left:0px; right:0px; top:0px; bottom:0px; background-color:#000; opacity:0.5"></div>\n' +
    ' <div style="'+style+'position:absolute; left:20px; right:20px; top:20px; bottom:20px; min-height:70px; border:1px solid #999; border-radius:6px; box-shadow: 0px 5px 15px rgba(0,0,0,0.5);">\n' +
    '  <div style="'+style+'position:absolute; width:100%; top:0px; height:30px; background-color:#f00; line-height:30px; text-align:center">\n' +
    '   <span style="'+style+'font-weight:bold; font-size:20px">Korektor Spellchecker</span>\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; width:100%; top:30px; bottom:30px; background-color:#0f0; overflow-x:hidden; overflow-y:auto; ">\n' +
    '   Eu congue metus ligula sed justo. Suspendisse\n' +
    '   potenti. Donec sodales elementum turpis. Duis dolor elit, dapibus sed,\n' +
    '   placerat vitae, auctor sit amet, nunc. Donec nisl quam, hendrerit vitae,\n' +
    '   porttitor et, imperdiet id, quam. Quisque dolor. Nulla tincidunt, lacus id\n' +
    '   dapibus ullamcorper, turpis diam fringilla eros, quis aliquet dolor felis\n' +
    '   at lorem. Pellentesque et lacus. Vestibulum tempor lectus at est.\n' +
    '   Pellentesque habitant morbi tristique senectus et netus et malesuada fames\n' +
    '   ac turpis egestas. Sed vitae eros. Nulla pulvinar turpis eget nunc. Sed\n' +
    '   bibendum pellentesque nunc. Integer tristique, lorem ac faucibus tempor,\n' +
    '   lorem dolor mollis turpis, a consectetuer nunc justo ac nisl.\n' +
    '   <div style="'+style+'height: 5em;"></div>\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; width:100%; bottom:0px; height:36px; background-color:#00f;">\n' +
    '   <div style="'+style+'position:absolute; left:0px; width:50%; height:100%; line-height:36px; text-align:center">\n' +
    '    <input type="checkbox" id="korektorEditReportText" style="'+style+'cursor:pointer" checked>\n' +
    '    <label style="'+style+'cursor:pointer" id="korektorEditReportTextLabel">Report corrected text</label>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:50%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:4px; right:2px; top:4px; bottom:4px">\n' +
    '     <button style="'+style+'width:100%; height:100%" id="korektorEditOk">OK</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:75%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:2px; right:4px; top:4px; bottom:4px">\n' +
    '     <button style="'+style+'width:100%; height:100%" id="korektorEditCancel">Cancel</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '  </div>\n' +
    ' </div>\n' +
    '</div>\n'
  );
  jQuery('#korektorEditReportTextLabel').click(function() {
    jQuery('#korektorEditReportText').prop('checked', !jQuery('#korektorEditReportText').prop('checked'));
  });
  jQuery('#korektorEditOk').click(function() {
  });
  jQuery('#korektorEditCancel').click(function() {
    jQuery('#korektorEditDialog').remove();
  });
}
