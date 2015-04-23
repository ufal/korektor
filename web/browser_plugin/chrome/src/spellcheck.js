// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

function performSpellcheck(model, edit) {
  var control = document.activeElement;
  if (control) {
    if ("value" in control) {
      control.disabled = true;
      jQuery.ajax('https://lindat.mff.cuni.cz/services/korektor/api/correct',
                  {dataType: "json", data: {model: model, data: control.value}, type: "POST", success: function(json) {
        if ("result" in json)
          control.value = json.result;
        else
          alert(chrome.i18n.getMessage("korektor_error"));
      }, error: function(jqXHR, textStatus) {
        alert(chrome.i18n.getMessage("korektor_error"));
      }, complete: function() {
        control.disabled = false;
      }});
    }
  }
}
