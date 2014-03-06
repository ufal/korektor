<script type="text/javascript" charset="utf-8"><!--
  function callCorrector(model) {
    var text = jQuery('#input').val();
    jQuery('#error').hide().empty();
    jQuery('#output_header').text("Output (computing...)");

    jQuery.ajax('http://quest.ms.mff.cuni.cz/korektor/api/' + model,
                {dataType: "json", data: {data: text}, type: "POST", success: function(json) {
      var result = '';
      for (var i in json.result) {
        var token = json.result[i];
        if (token.length == 1) {
          result += token[0];
        } else if (token.length == 2) {
          result += "<span class='single'>" + token[1] + "</span>";
        } else if (token.length > 2) {
          result += "<span class='multiple'>" + token[1] + "<div style='visibility:hidden'>"
          for (var j in token)
            if (j > 0)
              result += token[j] + "<br />";
          result += "</div></span>";
        }
      }
      jQuery('#output').html(result);
    }, error: function(jqXHR, textStatus) {
      jQuery('#error').text("An error occurred: " + textStatus).show();
    }, complete: function() {
      jQuery('#output_header').text("Output (done)");
    }});
  }
--></script>
<style type="text/css"><!--
  #output span.single { color: #800 }
  #output span.multiple { position: relative; color: #800; text-decoration: underline }
  #output span.multiple:hover { color: #e00; }
  #output span div { position: absolute; left: 0; top: 1.5em; white-space: pre; z-index: 1; background-color: #ee4; padding: 3px }
  #output span:hover div { visibility: visible !important }
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
