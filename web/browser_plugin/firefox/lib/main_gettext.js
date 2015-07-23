function gettext(locale) {
  var messages = locale == 'cs' ? {
    "menu_korektor_czech": "Korektor - čeština",
    "menu_korektor_spellcheck": "Kontrola pravopisu",
    "menu_korektor_generate_diacritics": "Doplnění diakritiky",
    "menu_korektor_strip_diacritics": "Odebrání diakritiky",
    "menu_korektor_with_edit": "...",
    "menu_korektor_without_edit": "bez následné editace",
    "menu_korektor_plugin_about": "O rozšíření Korektor Spellchecker",
    "menu_korektor_about": "O Korektoru"
  } : {
    "menu_korektor_czech": "Korektor - Czech",
    "menu_korektor_spellcheck": "Spellcheck",
    "menu_korektor_generate_diacritics": "Generate diacritics",
    "menu_korektor_strip_diacritics": "Strip diacritics",
    "menu_korektor_with_edit": "...",
    "menu_korektor_without_edit": " without further editing",
    "menu_korektor_plugin_about": "About Korektor Spellchecker extension",
    "menu_korektor_about": "About Korektor"
  };

  return function(message) { return message in messages ? messages[message] : "<translation missing>"; };
};

exports.gettext = gettext;
