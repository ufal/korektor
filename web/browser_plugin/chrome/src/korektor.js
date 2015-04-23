// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

function korektorSpellcheck(info, tab) {
  if ("menuItemId" in info) {
    var model = info.menuItemId;
    var edit = false;
    if (model.lastIndexOf("with-edit_", 0) == 0) {
      model = model.substr(10);
      edit = true;
    }

    chrome.tabs.executeScript(null, { file: "jquery-2.1.3.min.js" }, function() {
      chrome.tabs.executeScript(null, { file: "spellcheck.js" }, function() {
        chrome.tabs.executeScript(null, { code: "performSpellcheck('" + model + "', " + (edit ? "true" : "false") + ")" });
      });
    });
  }
}

chrome.contextMenus.onClicked.addListener(korektorSpellcheck);

chrome.runtime.onInstalled.addListener(function() {
  chrome.contextMenus.create({id: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_czech"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "czech-spellchecker", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_spellcheck"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "czech-diacritics_generator", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "strip_diacritics", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_strip_diacritics"), contexts: ["editable"]});
  chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator", parentId: "korektor_czech", contexts: ["editable"]});
  chrome.contextMenus.create({id: "with-edit_czech-spellchecker", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_spellcheck")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "with-edit_czech-diacritics_generator", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "with-edit_strip_diacritics", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
});
