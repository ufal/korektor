<?php $main_page=basename(__FILE__); require('header.php') ?>

<?php require('about.html') ?>

<script type="text/javascript" charset="utf-8"><!--
  var model_sets = { // Supported model sets
    "czech-130202": [
      {task: "Spellcheck", model: "czech-spellchecker-130202"},
      {task: "Generate Diacritics", model: "czech-diacritics_generator-130202"},
      {task: "Strip Diacritics", model: "strip_diacritics-130202"}
    ]
  };
  var language_model_sets = { // List of model sets per language
    "czech": ["czech-130202"]
  };
  var models = {}; // List of service models
  var placeholders = { // Placeholders for service models
    "czech-spellchecker-130202": "Přílyš žluťoučky kůň ůpěl ďábelské ódi.",
    "czech-diacritics_generator-130202": "Prilis zlutoucky kun upel dabelske ody.",
    "strip_diacritics-130202": "Příliš žluťoučký kůň úpěl ďábelské ódy."
  };
  var suggestions; // Array of suggestions for choosing
  var text_original, text_korektor; // Texts for Corrected text submission

  function callKorektor() {
    var model = jQuery('input[name=task]:checked').val();
    if (!model) return;

    var text = jQuery('#input').val();
    if (!text) text = jQuery('#input').attr('placeholder');
    if (!text) text = "";

    jQuery('#submit_correction').hide();
    jQuery('#submit_correction_results').empty();
    jQuery('#submit').html('<span class="fa fa-cog"></span> Waiting for Results <span class="fa fa-cog"></span>');
    jQuery('#submit').prop('disabled', true);
    suggestions = [];
    jQuery.ajax('//lindat.mff.cuni.cz/services/korektor/api/suggestions',
                {dataType: "json", data: {model: model, data: text}, type: "POST", success: function(json) {
      var result = '';
      for (var i in json.result) {
        var token = json.result[i];
        if (token.length == 1) {
          result += token[0];
        } else if (token.length >= 2) {
          var suggestion = suggestions.length;
          result += "<span class='" + (token.length > 2 ? "multiple" : "single") + "' id='sugg" + suggestion + "' onmouseover='showSugg(" + suggestion + ")' onmouseout='hideSugg()'>" + token[1] + "</span>";
          suggestions.push(token);
        }
      }
      jQuery('#output').html(result);

      jQuery('#acknowledgements_title').show();
      var acknowledgements = "";
      for (var a in json.acknowledgements)
        acknowledgements += "<a href='" + json.acknowledgements[a] + "'>" + json.acknowledgements[a] + "</a><br/>";
      jQuery('#acknowledgements_text').html(acknowledgements).show();

      text_original = text;
      text_korektor = jQuery('#output').text();
      jQuery('#submit_correction').show();
    }, error: function(jqXHR, textStatus) {
      alert("An error occurred: " + textStatus);
    }, complete: function() {
      jQuery('#submit').html('<span class="fa fa-arrow-down"></span> Process Input <span class="fa fa-arrow-down"></span>');
      jQuery('#submit').prop('disabled', false);
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
                {dataType: "json", data: {original: text_original, korektor: text_korektor, corrected: text_corrected, origin: "lindat_demo"}, type: "POST", success: function(json) {
      jQuery('#submit_correction_results').addClass("text-success").text('Submitted, thanks.');
    }, error: function(jqXHR, textStatus) {
      jQuery('#submit_correction_results').addClass("text-danger").text('Cannot submit, sorry.');
    }});
  }

  function updateModels() {
    var language = jQuery('input[name=language]:checked').val();
    var models_list = "";
    for (var i in language_model_sets[language]) {
      var model_set_name = language_model_sets[language][i];
      var model_set = model_sets[model_set_name];
      var have_required_models = true;
      for (var j in model_set)
        have_required_models = have_required_models && model_set[j].model in models;
      if (have_required_models)
        models_list += "<option value='" + model_set_name + "'" + (models_list ? "" : " selected") + ">" + model_set_name + "</option>";
      }
    jQuery('#model').html(models_list);
    updateTasks();
  }

  function updateTasks() {
    var model_set = jQuery('#model :selected').text();
    var tasks_list = "";
    for (var i in model_sets[model_set]) {
      var task = model_sets[model_set][i];
      tasks_list += '<label class="btn btn-primary' + (tasks_list ? '' : ' active') + '"><input type="radio" name="task" value="' + task.model + '" onchange="updatePlaceholder()" autocomplete="off"' + (tasks_list ? '' : ' checked') + '>' + task.task + '</label>';
    }
    jQuery('#tasks').html(tasks_list);
    jQuery('#tasks_container').show();
    updatePlaceholder();
  }

  function updatePlaceholder() {
    var model = jQuery('input[name=task]:checked').val();
    jQuery('#input').attr('placeholder', model in placeholders ? placeholders[model] : '');
  }

  jQuery(document).ready(function() {
    jQuery.ajax('//lindat.mff.cuni.cz/services/korektor/api/models',
                {dataType: "json", success: function(json) {
      for (var i in json.models)
        models[json.models[i]] = json.models[i];
      updateModels();
    }, complete: function() {
      if (jQuery.isEmptyObject(models)) {
        jQuery('#error').text("Cannot obtain the list of models from the service.").show();
      }
    }});
  });
--></script>

<div class="panel panel-info">
  <div class="panel-heading"><strong>Service</strong></div>
  <div class="panel-body">
    <p>The service is freely available for testing. Respect the
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
      <div class="form-group row" id="tasks_container" style="display: none">
        <label class="col-sm-2 control-label">Task:</label>
        <div class="col-sm-10 btn-group" data-toggle="buttons" id="tasks">
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
