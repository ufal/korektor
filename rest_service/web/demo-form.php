<script type="text/javascript" charset="utf-8"><!--
  var suggestions;
  function callCorrector(model) {
    var text = jQuery('#input').val();
    jQuery('#error').hide().empty();
    jQuery('#output_header').text("Output (computing...)");

    jQuery('suggestions').hide();
    suggestions = [];
    jQuery.ajax('http://quest.ms.mff.cuni.cz/korektor/api/' + model,
                {dataType: "json", data: {data: text}, type: "POST", success: function(json) {
      var result = '';
      for (var i in json.result) {
        var token = json.result[i];
        if (token.length == 2 && token[0] == token[1]) token.splice(1, 1);
        if (token.length == 1) {
          result += token[0];
        } else if (token.length >= 2) {
          var suggestion = suggestions.length;
          result += "<span class='" + (token.length > 2 ? "multiple" : "single") + "' id='sugg" + suggestion + "' onmouseover='showSugg(" + suggestion + ")' onmouseout='hideSugg()'>" + token[1] + "</span>";
          suggestions.push(token);
        }
      }
      jQuery('#output').html(result);
    }, error: function(jqXHR, textStatus) {
      jQuery('#error').text("An error occurred: " + textStatus).show();
    }, complete: function() {
      jQuery('#output_header').text("Output (done)");
    }});
  }

  var hoveringSuggestions;
  var hideSuggestions;
  function overSugg() { hoveringSuggestions = true; }
  function outSugg() { hoveringSuggestions = false; hideSugg(); }

  function showSugg(suggestion) {
    clearTimeout(hideSuggestions);
    var dialog = jQuery('#suggestions').html('').show();
    var word = jQuery('#sugg' + suggestion);
    dialog.offset({left: word.offset().left, top: word.offset().top + word.height()});
    setSuggContent(suggestion);
  }

  function setSuggContent(suggestion) {
    var word = jQuery('#sugg' + suggestion).text();
    var html = "<b>Original</b>";
    for (var i in suggestions[suggestion]) {
      if (i == 1) html += "<br/><b>Suggestions</b>";
      var this_one = word == suggestions[suggestion][i];
      html += "<br/><span onclick='setSugg(" + suggestion + "," + i + ")'>" + (this_one ? "<b>" : "") + suggestions[suggestion][i] + (this_one ? "</b>" : "") + "</span>";
    }
    jQuery('#suggestions').html(html);
  }

  function hideSugg() {
    clearTimeout(hideSuggestions);
    hideSuggestions = setTimeout(function(){if (!hoveringSuggestions) jQuery('#suggestions').hide()}, 300);
  }

  function setSugg(suggestion, index) {
    jQuery('#sugg' + suggestion).text(suggestions[suggestion][index]);
    setSuggContent(suggestion);
  }
--></script>
<style type="text/css"><!--
  #output span.single { color: #800 }
  #output span.multiple { color: #800; text-decoration: underline }
  #suggestions { padding: 5px; border: 1px solid #990; background-color: #ee4; }
  #suggestions span { color: #800; text-decoration: underline; cursor: pointer; cursor: hand }
--></style>

<h3>Input</h3>
<p id="error" style="width: 95%; margin: auto; border: 1px solid gray; background-color: #D66; display: none"></p>
<table style="width: 95%; margin: auto">
  <tr><td colspan="3" style="text-align: center"><textarea id="input" rows="10" cols="80" style="border: 1px solid gray; padding: 0; ma
rgin: 0; width: 100%" autofocus>Přílyš žluťoučky kůň ůpěl ďábelské ódi.</textarea></td></tr>
  <tr>
      <td style="text-align: center"><button type="submit" class="btn btn-primary" style="width: 100%" onclick="callCorrector('auto_correct')">Autocorrect</button></td>
      <td style="text-align: center"><button type="submit" class="btn btn-primary" style="width: 100%" onclick="callCorrector('strip_diacritics')">Strip Diacritics</button></td>
      <td style="text-align: center"><button type="submit" class="btn btn-primary" style="width: 100%" onclick="callCorrector('generate_diacritics')">Generate Diacritics</button></td>
</tr>
</table>

<h3 id="output_header">Output</h3>

<div style="width: 100%">
  <p id="output" style="white-space: pre-wrap; width: 95%; margin: auto; border: 1px solid gray; background-color: #6D6; min-height: 2em"></p>
</div>

<div id='suggestions' style='position: absolute; display: none' onmouseover='overSugg()' onmouseout='outSugg()'></div>
