// This file is part of korektor <https://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

async function korektorSpellcheck(tabId, message) {
  // Always inject — the content script guards against duplicate listeners,
  // so re-injection after a service worker restart is safe.
  await chrome.scripting.executeScript({
    target: { tabId: tabId },
    files: ["/spellcheck.js"]
  });
  chrome.tabs.sendMessage(tabId, message);
}

chrome.contextMenus.create({id: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_czech"), contexts: ["editable"]});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-czech-spellchecker",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_spellcheck")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-czech-diacritics_generator",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({
  id: "spellcheck_and_edit-strip_diacritics",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+chrome.i18n.getMessage("menu_korektor_with_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_1", parentId: "korektor_czech", contexts: ["editable"]});
chrome.contextMenus.create({
  id: "spellcheck-czech-spellchecker",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_spellcheck")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({
  id: "spellcheck-czech-diacritics_generator",
  parentId: "korektor_czech", title: chrome.i18n.getMessage("menu_korektor_generate_diacritics")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({
  id: "spellcheck-strip_diacritics",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_strip_diacritics")+chrome.i18n.getMessage("menu_korektor_without_edit"),
  contexts: ["editable"],
});
chrome.contextMenus.create({type: "separator", id: "korektor_czech_separator_2", parentId: "korektor_czech", contexts: ["editable"]});
chrome.contextMenus.create({
  id: "about_korektor_plugin",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_plugin_about"),
  contexts: ["editable"],
});
chrome.contextMenus.create({
  id: "about_korektor",
  parentId: "korektor_czech",
  title: chrome.i18n.getMessage("menu_korektor_about"),
  contexts: ["editable"],
});

// Centralized click handler replacing per-item onclick callbacks (removed in MV3).
chrome.contextMenus.onClicked.addListener((info, tab) => {
  switch (info.menuItemId) {
    case "spellcheck_and_edit-czech-spellchecker":
      korektorSpellcheck(tab.id, { model: 'czech-spellchecker', edit: true });
      break;
    case "spellcheck_and_edit-czech-diacritics_generator":
      korektorSpellcheck(tab.id, { model: 'czech-diacritics_generator', edit: true });
      break;
    case "spellcheck_and_edit-strip_diacritics":
      korektorSpellcheck(tab.id, { model: 'strip_diacritics', edit: true });
      break;
    case "spellcheck-czech-spellchecker":
      korektorSpellcheck(tab.id, { model: 'czech-spellchecker', edit: false });
      break;
    case "spellcheck-czech-diacritics_generator":
      korektorSpellcheck(tab.id, { model: 'czech-diacritics_generator', edit: false });
      break;
    case "spellcheck-strip_diacritics":
      korektorSpellcheck(tab.id, { model: 'strip_diacritics', edit: false });
      break;
    case "about_korektor_plugin":
      chrome.tabs.create({url: "https://ufal.mff.cuni.cz/korektor/online#plugin"});
      break;
    case "about_korektor":
      chrome.tabs.create({url: "https://ufal.mff.cuni.cz/korektor"});
      break;
  }
});
