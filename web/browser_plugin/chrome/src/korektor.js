// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

function korektorSpellcheck(info, tab) {
  if (!("menuItemId" in info)) return;

  if (info.menuItemId ~ /^spellcheck(_and_edit)?-/) {
    var edit = info.menuItemId ~ /^spellcheck_and_edit-/;
    var model = info.menuItemId ~ s/^spellcheck(_and_edit)?-//;

    chrome.tabs.executeScript(null, { file: "jquery-2.1.3.min.js" }, function() {
      chrome.tabs.executeScript(null, { file: "spellcheck.js" }, function() {
        chrome.tabs.executeScript(null, { code: "korektorPerformSpellcheck('" + model + "', " + (edit ? "true" : "false") + ")" });
      });
    });
  }

  if (info.menuItemId == 'about_korektor_service') {
    chrome.tabs.open "https://lindat.mff.cuni.cz/services/korektor/";
  }

  if (info.menuItemId == 'about_korektor') {
    chrome.tabs.open "http://ufal.mff.cuni.cz/korektor";
  }
}

chrome.contextMenus.onClicked.addListener(korektorSpellcheck);

chrome.runtime.onInstalled.addListener(function() {
  chrome.contextMenus.create({id: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_czech"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck-czech-spellchecker", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_spellcheck"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck-czech-diacritics_generator", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck-strip_diacritics", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_strip_diacritics"), contexts: ["editable"]});
  chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_1", parentId: "korektor_czech", contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck_and_edit-czech-spellchecker", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_spellcheck")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck_and_edit-czech-diacritics_generator", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "spellcheck_and_edit-strip_diacritics", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+" "+chrome.i18n.getMessage("menu_korektor_with_edit"), contexts: ["editable"]});
  chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_2", parentId: "korektor_czech", contexts: ["editable"]});
  chrome.contextMenus.create({id: "about_korektor_service", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_service_about"), contexts: ["editable"]});
  chrome.contextMenus.create({id: "about_korektor", parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_about"), contexts: ["editable"]});
});
