function gettext(locale) {
  var messages = locale == 'cs' ? {
    "korektor_service_error": "Nelze kontaktovat službu Korektor.",
    "korektor_editable_control_not_found": "Nelze najít editační prvek, který by bylo možné použít.",
    "korektor_text_modified_error": "Obsah editačního prvku byl změněn, nahrazení nebude provedeno.",
    "korektor_dlg_ok": "OK",
    "korektor_dlg_cancel": "Storno",
    "korektor_dlg_report_text": "Odeslat nám provedené změny",
    "korektor_dlg_suggestions": "Návrhy",
    "korektor_dlg_suggestions_original": "Původní",
    "korektor_dlg_suggestions_custom": "Vlastní"
  } : {
    "korektor_service_error": "Cannot contact the Korektor service.",
    "korektor_editable_control_not_found": "Cannot find editable control to use.",
    "korektor_text_modified_error": "The text of the editable field has changed, not replacing it.",
    "korektor_dlg_ok": "OK",
    "korektor_dlg_cancel": "Cancel",
    "korektor_dlg_report_text": "Send us performed changes",
    "korektor_dlg_suggestions": "Suggestions",
    "korektor_dlg_suggestions_original": "Original",
    "korektor_dlg_suggestions_custom": "Custom"
  };

  return function(message) { return message in messages ? messages[message] : "<translation missing>"; };
};
