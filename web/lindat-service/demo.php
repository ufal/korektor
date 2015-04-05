<?php $main_page=basename(__FILE__); require('header.php') ?>

<?php require('about.html') ?>

<script type="text/javascript" charset="utf-8"><!--
  var models = {};

  var suggestions;
  var text_original;
  var text_korektor;

  function callCorrector(model) {
    var text = jQuery('#input').val();
    jQuery('#error').hide().empty();
    jQuery('#submit_correction').hide();
    jQuery('#submit_correction_results').empty();
    jQuery('#output_header').text("Output (computing...)");

    jQuery('suggestions').hide();
    suggestions = [];
    jQuery.ajax('//lindat.mff.cuni.cz/services/korektor/api/suggestions',
                {dataType: "json", data: {model: model, data: text}, type: "POST", success: function(json) {
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

      text_original = text;
      text_korektor = jQuery('#output').text();
      jQuery('#submit_correction').show();
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

  function submitCorrection() {
    text_corrected = jQuery('#output').text();

    jQuery('#submit_correction_results').removeClass().empty();
    jQuery.ajax('//lindat.mff.cuni.cz/services/korektor/log.php',
                {dataType: "json", data: {original: text_original, korektor: text_korektor, corrected: text_corrected}, type: "POST", success: function(json) {
      jQuery('#submit_correction_results').addClass("text-success").text('Submitted, thanks.');
    }, error: function(jqXHR, textStatus) {
      jQuery('#submit_correction_results').addClass("text-danger").text('Cannot submit, sorry.');
    }});
  }

  function updateModels() {
    var language = jQuery('input[name=language]:checked').val();
    var models_list = "";
    var models_list_map = {};
    for (var i in models)
      if (models[i].indexOf(language+"-") == 0) {
        var version = models[i].match(/-\d\d\d\d\d\d/);
        if (version) {
          version = version[0].substr(1);
          if (!(version in models_list_map)) {
            models_list_map[version] = 1;
            models_list += "<option value='" + version + "'" + (models_list ? "" : " selected") + ">" + language + "-" + version + "</option>";
          }
        }
      }
    jQuery('#model').html(models_list);
    updateMethods();
  }

  jQuery(document).ready(function() {
    jQuery.ajax('//lindat.mff.cuni.cz/services/korektor/api/models',
                {dataType: "json", success: function(json) {
      models = json.models;
      updateModels();
    }, complete: function() {
      if (jQuery.isEmptyObject(models)) {
        jQuery('#error').text("Cannot obtain the list of models from the service.").show();
      }
    }});
  });
--></script>

<div class="panel panel-info">
  <div class="panel-heading"><strong>Demo</strong></div>
  <div class="panel-body">
    <p>The demo is freely available for testing. Respect the
    <a href="http://creativecommons.org/licenses/by-nc-sa/3.0/">CC BY-NC-SA</a>
    licence of the models &ndash; <b>explicit written permission of the authors is
    required for any commercial exploitation of the system</b>. If you use the
    service, you agree that data obtained by us during such use can be used for further
    improvements of the systems at UFAL. If you perform corrections to the output (either
    by choosing other suggestions or by manually correcting the text), please use
    the <b>Submit corrected text</b> button to send the corrected text to us.
    All comments and reactions are welcome.</p>

    <div id="error" class="alert alert-danger" style="display: none"></div>

    <div class="form-horizontal">
      <div class="form-group row">
        <label class="col-sm-2 control-label">Model:</label>
        <div class="col-sm-10">
          <label class="radio-inline"><input name="language" type="radio" value="czech" onchange="updateModels()" checked />Czech</label>
        </div>
      </div>
      <div class="form-group row">
        <div class="col-sm-offset-2 col-sm-10">
          <select id="model" class="form-control" onchange="updateTasks()"></select>
        </div>
      </div>
      <div class="form-group row" id="task_container" style="display: none">
        <label class="col-sm-2 control-label">Task:</label>
        <div class="col-sm-10 btn-group" data-toggle="buttons">
          <label class="btn btn-primary active"><input type="radio" name="task" id="task_spellchecker" autocomplete="off" checked>Spellcheck</label>
          <label class="btn btn-primary"><input type="radio" name="task" id="task_diacritics_generator" autocomplete="off">Generate Diacritics</label>
          <label class="btn btn-primary"><input type="radio" name="task" id="task_strip_diacritics" autocomplete="off">Strip Diacritics</label>
        </div>
      </div>
    </div>

    <textarea id="input" class="form-control" rows="10" cols="80" placeholder="Přílyš žluťoučky kůň ůpěl ďábelské ódi." autofocus></textarea>

    <button id="submit" class="btn btn-primary form-control" type="submit" style="margin-top: 15px; margin-bottom: 15px" onclick="callKorektor()"><span class="fa fa-arrow-down"></span> Process Input <span class="fa fa-arrow-down"></span></button>

    <div class="panel panel-success" id="output_panel">
      <div class="panel-heading">
        <strong>Output (editable)</strong>
        <div class="pull-right" style="display: none" id="submit_correction">
          <span id="submit_correction_results"></span>
          <button type="button" class="btn btn-primary btn-xs" style="padding: 0 2em" onclick="submitCorrection()">Submit corrected text</button>
        </div>
      </div>
      <div class="panel-body">
        <p id="output" style="white-space: pre-wrap" contenteditable="true"></p>
        <div id='suggestions' style='position: absolute; display: none' onmouseover='overSugg()' onmouseout='outSugg()'></div>
      </div>
    </div>

    <h3 id="acknowledgements_title" style="display: none">Acknowledgements</h3>
    <p id="acknowledgements_text" style="display: none"> </p>
  </div>
</div>

<?php require('footer.php') ?>
