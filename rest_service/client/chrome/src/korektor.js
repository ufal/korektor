// Copyright 2014 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

function onKorektor(info) {
  var models = {
    spellcheck: "czech-spellchecker",
    generate_diacritics: "czech-diacritics_generator",
    strip_diacritics: "strip_diacritics"
  };

  if (info.menuItemId in models)
    chrome.tabs.executeScript({
      code: 'if (document.activeElement && "value" in document.activeElement) {' +
            '  var c = document.activeElement;' +
            '  var xhr = new XMLHttpRequest();' +
            '  xhr.open("POST", "http://lindat.mff.cuni.cz/services/korektor/api/correct", true);' +
            '  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");' +
            '  xhr.responseType = "text";' +
            '  xhr.onreadystatechange = function(a,e) {' +
            '    if (xhr.readyState == 4) {' +
            '      try {' +
            '        if (xhr.status == 200 && xhr.response) {' +
            '          var response = JSON.parse(xhr.response);' +
            '          if ("result" in response) {' +
            '            c.value = response.result;'+
            '            c.disabled = false;' +
            '            return;' +
            '          }' +
            '        }' +
            '      } catch(e) {}' +
            '      c.disabled = false;' +
            '      alert("' + chrome.i18n.getMessage("korektor_error") + '");' +
            '    }' +
            '  };' +
            '  c.disabled = true; ' +
            '  xhr.send("model=' + models[info.menuItemId] + '&data=" + encodeURIComponent(c.value));' +
            '}'
    });
};

chrome.contextMenus.onClicked.addListener(onKorektor);

chrome.runtime.onInstalled.addListener(function() {
  chrome.contextMenus.create({id: "parent", title: chrome.i18n.getMessage("menu_korektor"), contexts: ["editable"]})
  chrome.contextMenus.create({id: "spellcheck", parentId: "parent", title: chrome.i18n.getMessage("menu_korektor_spellcheck"), contexts: ["editable"]})
  chrome.contextMenus.create({id: "generate_diacritics", parentId: "parent", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics"), contexts: ["editable"]})
  chrome.contextMenus.create({id: "strip_diacritics", parentId: "parent", title: chrome.i18n.getMessage("menu_korektor_strip_diacritics"), contexts: ["editable"]})
});
