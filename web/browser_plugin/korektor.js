// This file is part of korektor <https://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

// Keep tab ids with injected code here.
const injectedTabs = new Set();

function korektorSpellcheck(tabId, message) {
  // Inject the code only once to avoid multiple message listeners.
  if (!injectedTabs.has(tabId)) {
    chrome.tabs.executeScript(tabId, { file: "/jquery-2.1.3.min.js" }, function() {
      chrome.tabs.executeScript(tabId, { file: "/spellcheck.js" }, function() {
        injectedTabs.add(tabId);
        chrome.tabs.sendMessage(tabId, message);
      });
    });
  } else {
    chrome.tabs.sendMessage(tabId, message);
  }
}

// If a tab is reloaded, the code is no longer injected there.
chrome.tabs.onUpdated.addListener((tabId, changeInfo, tab) => {
  if (changeInfo.status === 'loading') {
    injectedTabs.delete(tabId);
  }
});

chrome.contextMenus.create({id: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_czech"), contexts: ["editable"]});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-czech-spellchecker",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_spellcheck")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'czech-spellchecker', edit: true }),
});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-czech-diacritics_generator",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'czech-diacritics_generator', edit: true }),
});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-strip_diacritics",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'strip_diacritics', edit: true }),
});
chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_1", parentId: "korektor_czech", contexts: ["editable"]});
chrome.contextMenus.create({
  id: "spellcheck-czech-spellchecker",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_spellcheck")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'czech-spellchecker', edit: false }),
});
chrome.contextMenus.create({
  id: "spellcheck-czech-diacritics_generator",
  parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'czech-diacritics_generator', edit: false }),
});
chrome.contextMenus.create({
  id: "spellcheck-strip_diacritics",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
  onclick: (info, tab) => korektorSpellcheck(tab.id, { model: 'strip_diacritics', edit: false }),
});
chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_2", parentId: "korektor_czech", contexts: ["editable"]});
chrome.contextMenus.create({
  id: "about_korektor_plugin",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_plugin_about"),
  contexts: ["editable"],
  onclick: (info, tab) => chrome.tabs.create({url: "https://ufal.mff.cuni.cz/korektor/online#plugin"}),
});
chrome.contextMenus.create({
  id: "about_korektor",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_about"),
  contexts: ["editable"],
  onclick: (info, tab) => chrome.tabs.create({url: "https://ufal.mff.cuni.cz/korektor"}),
});
