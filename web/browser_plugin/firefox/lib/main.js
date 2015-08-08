var data = require("sdk/self").data;
var tabs = require("sdk/tabs");

var locale = require("sdk/l10n").get("locale");
locale = locale == 'cs' ? 'cs' : 'en'; // Make sure we a) have reasonable default, b) sanitize locale value as we put it to content scripts
var gettext = require("./main_gettext").gettext(locale); // We do not use sdk/l10n because we cannot specify default locale

var cm = require("sdk/context-menu");
cm.Menu({
  label: gettext("menu_korektor_czech"),
  image: data.url("korektor_48.png"),
  context: cm.SelectorContext("input, textarea, [contenteditable='true'], [contenteditable='']"),
  contentScriptFile: [data.url("jquery-2.1.3.min.js"), data.url("spellcheck.js"), data.url("spellcheck_gettext.js")],
  contentScript: 'self.on("click", function(node, data) { korektorPerformSpellcheck(gettext("'+locale+'"), node, data.replace(/-editing$/, ""), data.search(/-editing$/) != -1); });',
  items: [
    cm.Item({label: gettext("menu_korektor_spellcheck")+gettext("menu_korektor_with_edit"), data: "czech-spellchecker-editing"}),
    cm.Item({label: gettext("menu_korektor_generate_diacritics")+gettext("menu_korektor_with_edit"), data: "czech-diacritics_generator-editing"}),
    cm.Item({label: gettext("menu_korektor_strip_diacritics")+gettext("menu_korektor_with_edit"), data: "strip_diacritics-editing"}),
    cm.Separator(),
    cm.Item({label: gettext("menu_korektor_spellcheck")+gettext("menu_korektor_without_edit"), data: "czech-spellchecker"}),
    cm.Item({label: gettext("menu_korektor_generate_diacritics")+gettext("menu_korektor_without_edit"), data: "czech-diacritics_generator"}),
    cm.Item({label: gettext("menu_korektor_strip_diacritics")+gettext("menu_korektor_without_edit"), data: "strip_diacritics"}),
    cm.Separator(),
    cm.Item({label: gettext("menu_korektor_plugin_about"), contentScript: 'self.on("click", function (node, data) { self.postMessage(); });', onMessage: function () { tabs.open("http://ufal.mff.cuni.cz/korektor/online#plugin"); }}),
    cm.Item({label: gettext("menu_korektor_about"), contentScript: 'self.on("click", function (node, data) { self.postMessage(); });', onMessage: function () { tabs.open("http://ufal.mff.cuni.cz/korektor"); }})
  ]
});
