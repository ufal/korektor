// This file is part of korektor <https://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

function korektorGetText(control) {
  // Input and textarea fields
  if ("value" in control) {
    var text = control.value;
    var selection = null;
    if ("selectionStart" in control && "selectionEnd" in control) {
      var start = control.selectionStart;
      var end = control.selectionEnd;
      if (start < end) selection = {start:start, end:end};
    }
    return {type:"value", control:control, text:text, whiteSpace:"pre-wrap", selection:selection};
  }

  // Contenteditable fields
  if (control.contentEditable == "true") {
    var text = '';
    var range = control.ownerDocument.getSelection();
    range = range != null && !range.isCollapsed && range.rangeCount == 1 ? range.getRangeAt(0) : null;
    var selection = {};

    function getText(node) {
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      var win = (node.ownerDocument || document).defaultView || window;
      var keep_newlines = win.getComputedStyle(node).whiteSpace.search(/^pre/i) != -1;
      if (range && range.startContainer == node && range.startOffset == 0) selection.start = text.length;
      if (range && range.endContainer == node && range.endOffset == 0) selection.end = text.length;
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3) {
          if (range && range.startContainer == child) selection.start = text.length + range.startOffset;
          if (range && range.endContainer == child) selection.end = text.length + range.endOffset;
          var child_text = child.textContent;
          text += keep_newlines ? child_text : child_text.replace(/[\r\n]/g, ' ');
        } else if (child.nodeType == 1 && child.nodeName.search(/^(input|textarea|script)$/i) == -1 && child.contentEditable != "false") {
          var childWin = (child.ownerDocument || document).defaultView || window;
          var block_display = childWin.getComputedStyle(child).display.search(/^(block|list-item|table.*)$/i) != -1;

          if (range && range.startContainer == node && range.startOffset == i) selection.start = text.length;
          if (range && range.endContainer == node && range.endOffset == i) selection.end = text.length;
          getText(child);
          if (block_display || child.nodeName.search(/^br$/i) != -1) text += block_display ? "\n\n" : "\n";
          if (range && range.startContainer == node && range.startOffset == i+1) selection.start = text.length;
          if (range && range.endContainer == node && range.endOffset == i+1) selection.end = text.length;
        }
      }
    }
    getText(control);
    var have_selection = range && "start" in selection && "end" in selection;
    var ctrlWin = (control.ownerDocument || document).defaultView || window;
    return {type:"contentEditable", control:control, text:text, whiteSpace:ctrlWin.getComputedStyle(control).whiteSpace, selection:have_selection ? selection : null};
  }

  return null;
}

function korektorProcessResults(data, textArray) {
  // Remove empty elements from textArray
  for (var i = 0; i < textArray.length; i++)
    if (textArray[i].length == 0)
      textArray.splice(i--, 1);

  // Remove identical replacements from textArray
  for (var i = 0; i < textArray.length; i++)
    if (textArray[i].length > 1 && textArray[i][1] == textArray[i][0])
      textArray[i].splice(1, textArray[i].length - 1)
}

function korektorSetText(gettext, data, textArray) {
  // Check that current value is still the same
  var new_data = korektorGetText(data.control);
  if (!new_data || new_data.text != data.text) {
      alert(gettext('korektor_text_modified_error'));
      return;
    }

  // Input and textarea fields
  if (data.type == "value") {
    // Concatenate new text
    var text = '';
    for (var i = 0; i < textArray.length; i++)
      text += textArray[i][textArray[i].length > 1 ? 1 : 0];

    // Replace the text in the control and update selection if any
    if (data.selection) {
      data.control.value = data.text.substr(0, data.selection.start) + text + data.text.substr(data.selection.end);
      data.control.selectionStart = data.selection.start;
      data.control.selectionEnd = data.selection.start + text.length;
    } else
      data.control.value = text;
  }

  // Contenteditable fields
  if (data.type == "contentEditable") {
    var index = 0;
    var range = data.selection ? document.createRange() : null;
    function setText(node) {
      if (!("childNodes" in node)) return;
      var children = node.childNodes;
      if (data.selection && data.selection.start == 0) range.setStart(node, 0);
      if (data.selection && data.selection.end == 0) range.setEnd(node, 0);
      for (var i = 0; i < children.length; i++) {
        var child = children[i];
        if (child.nodeType == 3) {
          var prev_all = child.textContent, prev = prev_all, prev_prefix_len = -1, prev_suffix_index = -1;
          if (data.selection && data.selection.end-index >= 0 && data.selection.end-index <= prev_all.length) { prev_suffix_index = data.selection.end - index; prev = prev.substr(0, prev_suffix_index); }
          if (data.selection && data.selection.start-index >= 0 && data.selection.start-index <= prev_all.length) { prev_prefix_len = data.selection.start - index; prev = prev.substr(prev_prefix_len); }
          var prev_ori = prev, next = '';

          if (!data.selection || (data.selection.start < index + prev_all.length && data.selection.end > index)) while (prev) {
            while (textArray.length && !textArray[0][0]) textArray.shift();
            if (!textArray) break;
            var len = prev.length < textArray[0][0].length ? prev.length : textArray[0][0].length;
            if (textArray[0].length == 1) {
              next += prev.substr(0, len);
            } else if (textArray[0].length > 1 && textArray[0][1]) {
              next += textArray[0][1];
              textArray[0][1] = "";
            }
            prev = prev.substr(len);
            textArray[0][0] = textArray[0][0].substr(len);
          }
          next += prev;

          if (prev_prefix_len >= 0) next = prev_all.substr(0, prev_prefix_len) + next;
          if (prev_suffix_index >= 0) next = next + prev_all.substr(prev_suffix_index);
          if (next != prev_all) child.textContent = next;
          index += prev_all.length;
          if (prev_prefix_len >= 0) range.setStart(child, prev_prefix_len);
          if (prev_suffix_index >= 0) range.setEnd(child, prev_suffix_index + next.length - prev_all.length);
        } else if (child.nodeType == 1 && child.nodeName.search(/^(input|textarea|script)$/i) == -1 && child.contentEditable != "false") {
          var childWin = (child.ownerDocument || document).defaultView || window;
          var block_display = childWin.getComputedStyle(child).display.search(/^(block|list-item|table.*)$/i) != -1;

          if (data.selection && data.selection.start == index) range.setStart(node, i);
          if (data.selection && data.selection.end == index) range.setEnd(node, i);
          setText(child);
          if (block_display || child.nodeName.search(/^br$/i) != -1)
            for (var newlines = block_display ? 2 : 1; newlines > 0; newlines--) {
              if (!data.selection || (data.selection.start < index + 1 && data.selection.end > index)) {
                while (textArray.length && !textArray[0][0]) textArray.shift();
                if (textArray.length && textArray[0][0][0] == "\n") textArray[0][0] = textArray[0][0].substr(1);
              }
              index++;
            }
          if (data.selection && data.selection.start == index) range.setStart(node, i+1);
          if (data.selection && data.selection.end == index) range.setEnd(node, i+1);
        }
      }
    }
    setText(data.control);
    if (data.selection) {
      var selection = data.control.ownerDocument.getSelection();
      if (selection) {
        selection.removeAllRanges();
        selection.addRange(range);
      }
    }
  }
  data.control.focus();
}

function korektorReport(data, korektorArray, correctedArray) {
  var original = data.selection ? data.text.substring(data.selection.start, data.selection.end) : data.text;
  var korektor = '';
  for (var i = 0; i < korektorArray.length; i++)
    korektor += korektorArray[i][korektorArray[i].length > 1 ? 1 : 0];
  var corrected = '';
  for (var i = 0; i < correctedArray.length; i++)
    corrected += correctedArray[i][correctedArray[i].length > 1 ? 1 : 0];

  var params = new URLSearchParams({
    original: original,
    korektor: korektor,
    corrected: corrected,
    origin: "korektor_plugin"
  });

  fetch('https://lindat.mff.cuni.cz/services/korektor/log.php', {
    method: 'POST',
    body: params
  }).catch(function() {});
}

function korektorPerformSpellcheck(gettext, model, edit) {
  // Find the editable control. Note that
  // when the contenteditable is inside an iframe, document.activeElement
  // contains only the iframe, so we need to hop over it ourselves.
  var control = document.activeElement;
  while (control && control.contentDocument && control.contentDocument.activeElement && control.contentDocument.activeElement != control)
    control = control.contentDocument.activeElement;

  // Abort if no control was given
  if (!control) {
    alert(gettext('korektor_editable_control_not_found'));
    return;
  }

  // In Firefox, if control is in an iframe, window refers to its window.
  // Therefore, explicitly use the document of the top window.
  var topDocument = window.top.document;

  // Ignore if edit is in progress
  if (topDocument.getElementById('korektorEditDialogFrame')) return;

  // Try getting original text
  var data = korektorGetText(control);
  if (!data) {
    alert(gettext('korektor_editable_control_not_found'));
    return;
  }

  // Run Korektor
  data.topDocument = topDocument;
  var params = new URLSearchParams({
    model: model,
    data: data.selection ? data.text.substring(data.selection.start, data.selection.end) : data.text,
    suggestions: edit ? 5 : 1
  });

  fetch('https://lindat.mff.cuni.cz/services/korektor/api/suggestions', {
    method: 'POST',
    body: params
  })
  .then(function(response) {
    if (!response.ok) throw new Error('HTTP error ' + response.status);
    return response.json();
  })
  .then(function(json) {
    if (!("result" in json)) {
      alert(gettext('korektor_service_error'));
      return;
    }
    var textArray = [];
    for (var i = 0; i < json.result.length; i++)
      if (json.result[i].length)
        textArray.push(json.result[i]);

    // Process the results
    korektorProcessResults(data, textArray);

    // Edit result or use it directly
    if (edit)
      korektorEdit(gettext, data, textArray);
    else
      korektorSetText(gettext, data, textArray);
  })
  .catch(function() {
    alert(gettext('korektor_service_error'));
  });
}

function korektorEdit(gettext, data, textArray) {
  var styleWithoutFont = 'z-index:123456789; color:#000; background-color:transparent;';
  var style = styleWithoutFont + 'font-family:serif; font-size:16px;';

  // Fill suggestions
  function fillSuggestions(data, textArray) {
    // Check that current value is still the same
    var new_data = korektorGetText(data.control);
    if (!new_data || new_data.text != data.text) {
        alert(gettext('korektor_text_modified_error'));
        return;
      }

    var container = document.createElement("div");
    function suggestionSpan(index) {
      var span = document.createElement("span");
      span.setAttribute("style", styleWithoutFont+'color:#800;'+(textArray[index].length > 2 ? 'text-decoration: underline;' : ''));
      span.setAttribute("id", "korektorEditSuggestion" + index);
      return span;
    }

    if (data.type == "value") {
      for (var i = 0; i < textArray.length; i++)
        if (textArray[i].length == 1)
          container.appendChild(document.createTextNode(textArray[i][0]));
        else
          container.appendChild(suggestionSpan(i)).appendChild(document.createTextNode(textArray[i][1]));
    } else if (data.type == "contentEditable") {
      // Copy textArray
      var suggestions = [];
      for (var i = 0; i < textArray.length; i++)
        suggestions.push(textArray[i].length == 1 ? [textArray[i][0]] : [textArray[i][0], textArray[i][1]]);
      // Process node and create suggestion spans
      var index = 0;
      function process(node, copy) {
        if (!("childNodes" in node)) return;
        var children = node.childNodes;
        for (var i = 0; i < children.length; i++) {
          var child = children[i];
          if (child.nodeType == 3) {
            var text_all = child.textContent, text = text_all, next = '';
            if (data.selection && data.selection.end-index >= 0 && data.selection.end-index <= text_all.length) text = text.substr(0, data.selection.end - index);
            if (data.selection && data.selection.start-index >= 0 && data.selection.start-index <= text_all.length) text = text.substr(data.selection.start - index);

            if (!data.selection || (data.selection.start < index + text_all.length && data.selection.end > index)) while (text) {
              while (suggestions.length && !suggestions[0][0]) suggestions.shift();
              if (!suggestions) break;
              var len = text.length < suggestions[0][0].length ? text.length : suggestions[0][0].length;
              if (suggestions[0].length == 1) {
                next += text.substr(0, len);
              } else if (suggestions[0].length > 1 && suggestions[0][1]) {
                if (next.length) copy.appendChild(document.createTextNode(next));
                copy.appendChild(suggestionSpan(textArray.length - suggestions.length)).appendChild(document.createTextNode(suggestions[0][1]));
                next = '';
                suggestions[0][1] = '';
              }
              text = text.substr(len);
              suggestions[0][0] = suggestions[0][0].substr(len);
            }
            if (next.length) {
              copy.appendChild(document.createTextNode(next));
              nonempty = true;
            }

            index += text_all.length;
          } else if (child.nodeType == 1 && child.nodeName.search(/^(input|textarea|script)$/i) == -1 && child.contentEditable != "false") {
            var childWin = (child.ownerDocument || document).defaultView || window;
            var block_display = childWin.getComputedStyle(child).display.search(/^(block|list-item|table.*)$/i) != -1;

            var start_index = index;
            var childCopy = "cloneNode" in child ? child.cloneNode(false) : null;
            process(child, childCopy ? childCopy : copy);
            if (!data.selection || (data.selection.start < index && data.selection.end > start_index))
              if (childCopy) copy.appendChild(childCopy);

            if (block_display || child.nodeName.search(/^br$/i) != -1)
              for (var newlines = block_display ? 2 : 1; newlines > 0; newlines--) {
                if (!data.selection || (data.selection.start < index + 1 && data.selection.end > index)) {
                  while (suggestions.length && !suggestions[0][0]) suggestions.shift();
                  if (suggestions.length && suggestions[0][0][0] == "\n") suggestions[0][0] = suggestions[0][0].substr(1);
                  nonempty = true;
                }
                index++;
              }
          }
        }
      }
      process(data.control, container);
    }
    return container.innerHTML +
      '<div style="'+style+'height: 5em;"></div>' +
      '<div style="'+style+'position:absolute; background-color:#eee; padding:10px; border:1px solid #999; border-radius:6px; box-shadow: 0px 5px 15px rgba(0,0,0,0.5); display:none" id="korektorEditSuggestions"></div>';
  }
  var html = fillSuggestions(data, textArray);
  if (!html) return;

  // Add dialog
  var frame = data.topDocument.createElement('iframe');
  frame.setAttribute('src', 'javascript:""');
  frame.setAttribute('width', '100%');
  frame.setAttribute('height', '100%');
  frame.setAttribute('style', style + 'display:block; position:fixed; left:0px; right:0px; top:0px; bottom:0px');
  frame.setAttribute('id', 'korektorEditDialogFrame');
  frame.setAttribute('tabindex', '-1');
  frame.setAttribute('contenteditable', 'false');
  data.topDocument.body.appendChild(frame);

  data.dlgDocument = frame.contentDocument || frame.contentWindow.document;
  data.dlgDocument.body.innerHTML =
    '<div style="'+style+'position:fixed; left:0px; right:0px; top:0px; bottom:0px" id="korektorEditDialog" tabindex="-1" contenteditable="false">\n' +
    ' <div style="'+style+'position:absolute; left:0px; right:0px; top:0px; bottom:0px; background-color:#000; opacity:0.5"></div>\n' +
    ' <div style="'+style+'position:absolute; left:20px; right:20px; top:20px; bottom:20px; min-height:80px; background-color:#eee; border:1px solid #999; border-radius:6px; box-shadow: 0px 5px 15px rgba(0,0,0,0.5);">\n' +
    '  <div style="'+style+'position:absolute; width:100%; top:0px; height:36px; line-height:36px; text-align:center">\n' +
    '   <span style="'+style+'font-weight:bold; font-size:20px">Korektor Spellchecker</span>\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; left:8px; right:8px; top:36px; bottom:44px; background-color:#fff; border-radius:4px; border:1px solid #999; overflow-x: hidden; overflow-y: visible" id="korektorEditText">\n' +
    '  </div>\n' +
    '  <div style="'+style+'position:absolute; width:100%; bottom:0px; height:44px;">\n' +
    '   <div style="'+style+'position:absolute; left:0px; width:50%; height:100%; line-height:44px; text-align:center">\n' +
    '    <input type="checkbox" id="korektorEditReportText" style="'+style+'cursor:pointer" checked>\n' +
    '    <label style="'+style+'cursor:pointer" id="korektorEditReportTextLabel">'+gettext('korektor_dlg_report_text')+'</label>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:50%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:8px; right:4px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditOk">'+gettext('korektor_dlg_ok')+'</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '   <div style="'+style+'position:absolute; left:75%; width:25%; height:100%">\n' +
    '    <div style="'+style+'position:absolute; left:4px; right:8px; top:8px; bottom:8px">\n' +
    '     <button style="'+style+'width:100%; height:100%; background-color: #ddd; border:1px solid #999; border-radius:4px" id="korektorEditCancel">'+gettext('korektor_dlg_cancel')+'</button>\n' +
    '    </div>\n' +
    '   </div>\n' +
    '  </div>\n' +
    ' </div>\n' +
    '</div>';

  // Dialog controls
  function korektorEditDialogClose() {
    var dlgFrame = data.topDocument.getElementById('korektorEditDialogFrame');
    if (dlgFrame) dlgFrame.remove();
    data.control.focus();
  }
  function korektorEditDialogHandleEscape(event) {
    var dlgFrame = data.topDocument.getElementById('korektorEditDialogFrame');
    if (dlgFrame && event.keyCode == 27) korektorEditDialogClose();
  }
  data.dlgDocument.addEventListener('keyup', korektorEditDialogHandleEscape);

  var reportTextLabel = data.dlgDocument.getElementById('korektorEditReportTextLabel');
  if (reportTextLabel) {
    reportTextLabel.addEventListener('click', function() {
      var reportText = data.dlgDocument.getElementById('korektorEditReportText');
      if (reportText) reportText.checked = !reportText.checked;
    });
  }

  var okBtn = data.dlgDocument.getElementById('korektorEditOk');
  if (okBtn) {
    okBtn.addEventListener('click', function() {
      var resultArray = [];
      for (var i = 0; i < textArray.length; i++) {
        var sugNode = data.dlgDocument.getElementById('korektorEditSuggestion' + i);
        var sugText = sugNode ? sugNode.textContent : '';
        resultArray.push(textArray[i].length == 1 ? textArray[i] : [textArray[i][0], sugText]);
      }
      var reportText = data.dlgDocument.getElementById('korektorEditReportText');
      if (reportText && reportText.checked) korektorReport(data, textArray, resultArray);
      korektorSetText(gettext, data, resultArray);
      korektorEditDialogClose();
    });
  }

  var cancelBtn = data.dlgDocument.getElementById('korektorEditCancel');
  if (cancelBtn) {
    cancelBtn.addEventListener('click', korektorEditDialogClose);
  }

  // Suggestion dialog handling
  var korektorEditSuggestionsHovering = false;
  var korektorEditSuggestionsHideTimeout = null;
  function korektorEditSuggestionsMouseEnter() {
    korektorEditSuggestionsHovering = true;
  }
  function korektorEditSuggestionsMouseLeave() {
    korektorEditSuggestionsHovering = false;
    if (korektorEditSuggestionsHideTimeout !== null) clearTimeout(korektorEditSuggestionsHideTimeout);
    korektorEditSuggestionsHideTimeout = setTimeout(function(){
      korektorEditSuggestionsHideTimeout = null;
      if (!korektorEditSuggestionsHovering) {
        var sugBox = data.dlgDocument.getElementById('korektorEditSuggestions');
        if (sugBox) sugBox.style.display = 'none';
      }
    }, 300);
  }
  function korektorEditSuggestionsUpdate(suggestion) {
    var suggestion_text = suggestion.textContent;
    var spans = data.dlgDocument.querySelectorAll('#korektorEditSuggestions span');
    for (var i = 0; i < spans.length; i++) {
      spans[i].style.fontWeight = (spans[i].textContent == suggestion_text ? 'bold' : 'normal');
    }
  }
  function korektorEditSuggestionsFill(suggestion) {
    var id = suggestion.id.replace(/^korektorEditSuggestion/, '');

    var html = '<b>'+gettext('korektor_dlg_suggestions_original')+'</b>';
    var quoter = document.createElement('div');
    for (var i = 0; i < textArray[id].length; i++) {
      if (i == 1) html += '<br/><b>'+gettext('korektor_dlg_suggestions')+'</b>';
      quoter.textContent = textArray[id][i];
      html += '<br><span style="'+style+'color:#800; text-decoration:underline; cursor:pointer;">' + quoter.innerHTML + '</span>';
    }
    quoter.textContent = suggestion.textContent;
    html += '<br/><b>'+gettext('korektor_dlg_suggestions_custom')+'</b><br/><input type="text" style="'+style+'background:#fff; border:1px solid #999; border-radius:4px; width: 100%" value="' + quoter.innerHTML.replace(/"/g, '&quot;') + '"/>';

    var sugBox = data.dlgDocument.getElementById('korektorEditSuggestions');
    sugBox.innerHTML = html;
    sugBox.style.display = 'block';

    var rect = suggestion.getBoundingClientRect();
    var win = data.dlgDocument.defaultView || window;
    var scrollLeft = win.pageXOffset || data.dlgDocument.documentElement.scrollLeft || data.dlgDocument.body.scrollLeft || 0;
    var scrollTop = win.pageYOffset || data.dlgDocument.documentElement.scrollTop || data.dlgDocument.body.scrollTop || 0;

    sugBox.style.left = (rect.left + scrollLeft) + 'px';
    sugBox.style.top = (rect.bottom + scrollTop) + 'px';

    korektorEditSuggestionsUpdate(suggestion);

    var sugSpans = sugBox.querySelectorAll('span');
    for (var i = 0; i < sugSpans.length; i++) {
      sugSpans[i].addEventListener('click', function() {
        suggestion.textContent = this.textContent;
        korektorEditSuggestionsUpdate(suggestion);
      });
    }

    var sugInput = sugBox.querySelector('input');
    if (sugInput) {
      var updateFromInput = function() {
        suggestion.textContent = this.value;
        korektorEditSuggestionsUpdate(suggestion);
      };
      sugInput.addEventListener('change', updateFromInput);
      sugInput.addEventListener('input', updateFromInput);
    }
  }
  function korektorEditSuggestionsShow() {
    korektorEditSuggestionsMouseEnter();
    korektorEditSuggestionsFill(this);
  }

  // Fill the dialog with suggestions
  try {
    var editText = data.dlgDocument.getElementById('korektorEditText');
    if (editText) {
      editText.style.whiteSpace = data.whiteSpace;
      editText.innerHTML = html;
    }
  } catch (e) {}

  var sugBox = data.dlgDocument.getElementById('korektorEditSuggestions');
  if (sugBox) {
    sugBox.addEventListener('mouseenter', korektorEditSuggestionsMouseEnter);
    sugBox.addEventListener('mouseleave', korektorEditSuggestionsMouseLeave);
  }

  var textSpans = data.dlgDocument.querySelectorAll('#korektorEditText span');
  for (var i = 0; i < textSpans.length; i++) {
    textSpans[i].addEventListener('mouseenter', korektorEditSuggestionsShow);
    textSpans[i].addEventListener('mouseleave', korektorEditSuggestionsMouseLeave);
  }

  var editDialog = data.dlgDocument.getElementById('korektorEditDialog');
  if (editDialog) editDialog.focus();
}

// Listen to messages from background to perform spellcheck.
// Guard against duplicate listeners from repeated script injection
// (the MV3 service worker re-injects on every invocation).
if (!window._korektorListenerRegistered) {
  window._korektorListenerRegistered = true;
  chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
    korektorPerformSpellcheck(chrome.i18n.getMessage, request.model, request.edit);
  });
}
