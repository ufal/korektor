<?php $main_page=basename(__FILE__); require('header.php') ?>

<div class="dropdown pull-right" style='margin-left: 10px; margin-bottom: 10px'>
  <button class="btn btn-default dropdown-toggle" type="button" id="tocDropdown" data-toggle="dropdown"><span class="fa fa-bars"></span> Table of Contents <span class="caret"></span></button>
  <ul class="dropdown-menu dropdown-menu-right" aria-labelledby="tocDropdown">
    <li><a href="#api_reference"> API Reference</a></li>
    <li><a href="#models"><span class="fa fa-caret-right"></span> <code>models</code></a></li>
    <li><a href="#correct"><span class="fa fa-caret-right"></span> <code>correct</code></a></li>
    <li><a href="#suggestions"><span class="fa fa-caret-right"></span> <code>suggestions</code></a></li>
    <li class="divider"></li>
    <li><a href="#response_format">Common Response Format</a></li>
    <li class="divider"></li>
    <li><a href="#model_selection">Model Selection</a></li>
    <li class="divider"></li>
    <li><a href="#using_curl">Accessing API using Curl</a></li>
  </ul>
</div>

<p>Korektor web service is available on
<code>http(s)://lindat.mff.cuni.cz/services/korektor/api/</code>.</p>

<p>The web service is freely available for testing. Respect the
<a href="http://creativecommons.org/licenses/by-nc-sa/3.0/">CC BY-NC-SA</a>
licence of the models &ndash; <b>explicit written permission of the authors is
required for any commercial exploitation of the system</b>. If you use the
service, you agree that data obtained by us during such use can be used for further
improvements of the systems at UFAL. All comments and reactions are welcome.</p>

<h2 id="api_reference">API Reference</h2>

<p>The Korektor REST API can be accessed directly or via any other web
programming tools that support standard HTTP request methods and JSON for output
handling.</p>

<table class='table table-striped table-bordered'>
<tr>
    <th>Service Request</th>
    <th>Description</th>
    <th>HTTP Method</th>
</tr>
<tr>
    <td><a href="#models">models</a></td>
    <td>return list of models</td>
    <td>GET/POST</td>
</tr>
<tr>
    <td><a href="#correct">correct</a></td>
    <td>correct given text according to chosen model</td>
    <td>GET/POST</td>
</tr>
<tr>
    <td><a href="#suggestions">suggestions</a></td>
    <td>generate spelling suggestions of the given text according to chosen model</td>
    <td>GET/POST</td>
</tr>
</table>

<h3>Method <a id='models'>models</a></h3>

<p>Return the list of available models. The default model (used when user
supplies no model to a method call) is also returned &ndash; this is guaranteed
to be the latest Czech spellchecking model.</p>

<h5>Browser Example</h5>

<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://lindat.mff.cuni.cz/services/korektor/api/models</pre></td>
     <td style='vertical-align: middle; width: 6em'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://lindat.mff.cuni.cz/services/korektor/api/models')">try&nbsp;this</button></td></tr>
</table>

<h4>JSON Response</h4>

<p>The response object contains two fields <code>models</code> (containing array
of existing model names) and <code>default_model</code> (one of the models which
is used when no model is specified).

<h5>Example JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "models": [
  "czech-spellchecker-130202"
 ,"czech-diacritics_generator-130202"
 ,"strip_diacritics-130202"
 ]
,"default_model": "czech-spellchecker-130202"
}
</pre>

<hr />

<h3>Method <a id='correct'>correct</a></h3>

<p>Auto-correct the given text according to chosen model and return the corrected text as a string. The response format is <a href="#response_format">described later</a>.</p>

<table class='table table-striped table-bordered'>
<tr><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
<tr><td>model</td><td>no</td><td>string</td><td>Model to use; see <a href="#model_selection">model selection</a> for model matching rules.</td></tr>
<tr><td>input</td><td>no</td><td>string (<code>untokenized</code> / <code>untokenized_lines</code> / <code>segmented</code> / <code>vertical</code> / <code>horizontal</code>)</td><td><a href="http://ufal.mff.cuni.cz/korektor/users-manual#korektor_input_formats">Input format to use</a>; default is <code>untokenized</code>.</td></tr>
</table>

<h5>Browser Examples</h5>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://lindat.mff.cuni.cz/services/korektor/api/correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.</pre></td>
     <td style='vertical-align: middle; width: 6em'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://lindat.mff.cuni.cz/services/korektor/api/correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.')">try&nbsp;this</button></td></tr>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://lindat.mff.cuni.cz/services/korektor/api/correct?data=Příliš žluťoučký kůň úpěl ďábelské ódy .&amp;input=horizontal&amp;model=strip_diacritics</pre></td>
     <td style='vertical-align: middle; width: 6em'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://lindat.mff.cuni.cz/services/korektor/api/correct?data=Příliš žluťoučký kůň úpěl ďábelské ódy .&amp;input=horizontal&amp;model=strip_diacritics')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h3>Method <a id='suggestions'>suggestions</a></h3>

<p>Generate spelling suggestions for the given text. For every located error,
a list of suggestions is returned, from the most probable to the least probable.
User can specify the limit on number of suggestions returned. The response
format is <a href="#response_format">described later</a>.</p>

<table class='table table-striped table-bordered'>
<tr><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
<tr><td>model</td><td>no</td><td>string</td><td>Model to use; see <a href="#model_selection">model selection</a> for model matching rules.</td></tr>
<tr><td>input</td><td>no</td><td>string (<code>untokenized</code> / <code>untokenized_lines</code> / <code>segmented</code> / <code>vertical</code> / <code>horizontal</code>)</td><td><a href="http://ufal.mff.cuni.cz/korektor/users-manual#korektor_input_formats">Input format to use</a>; default is <code>untokenized</code>.</td></tr>
<tr><td>suggestions</td><td>no</td><td>positive integer</td><td>The maximum number of suggestions to return for a single token. If unspecified, value 5 is used.</td></tr>
</table>

<h5>Result Object</h5>

<p>The <code>result</code> field in the <a href="#response_format">response
format</a> is an array of suggestions. Each suggestion is an array of strings,
whose first element is the original piece of text and the other elements (which
may or may not be present) are the suggestions, from the most probable to the
least probable. The concatenation of first elements of suggestions is equal to
the original text.</p>

<h6>Example JSON Response</h6>
<pre class="prettyprint lang-json">
{
 "model": "czech-spellchecker-130202"
,"acknowledgements": [
  "http://ufal.mff.cuni.cz/korektor#korektor_acknowledgements",
 ,"http://ufal.mff.cuni.cz/korektor/users-manual#korektor-czech_acknowledgements"
 ]
,"result": [["Přílyš","Příliš","Přílis"],[" "],["žluťoučky","žluťoučký","žluťoučké"],
  [" kůň "],["ůpěl","úpěl","pěl"],[" ďábelské "],["ódi","ódy","zdi"],["."]]
}
</pre>

<h5>Browser Examples</h5>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://lindat.mff.cuni.cz/services/korektor/api/suggestions?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://lindat.mff.cuni.cz/services/korektor/api/suggestions?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.')">try&nbsp;this</button></td></tr>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://lindat.mff.cuni.cz/services/korektor/api/suggestions?data=Prilis zlutoucky kun upel dabelske ody.&amp;model=czech-diacritics_generator&amp;suggestions=3</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://lindat.mff.cuni.cz/services/korektor/api/suggestions?data=Prilis zlutoucky kun upel dabelske ody.&amp;model=czech-diacritics_generator&amp;suggestions=3')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h2 id="response_format">Common Response Format</h2>

<p>The response format of all methods is
<a href="http://en.wikipedia.org/wiki/JSON">JSON</a>. Except for the
<a href="#models">models</a> method, the output JSON has the following structure
(with <code>result_object</code> being usually a string or an array):</p>

<pre class="prettyprint lang-json">
{
 "model": "Model used"
,"acknowledgements": ["URL with acknowledgements", ...]
,"result": result_object
}
</pre>

<hr />

<h2 id="model_selection">Model Selection</h2>

<p>There are several possibilities how to select required model using
the <code>model</code> option:</p>

<ul>
  <li>If <code>model</code> option is not specified, the default model
  (returned by <a href="#models">models</a> method) is used &ndash; this is
  guaranteed to be the latest Czech spellcheching model.</li>

  <li>The <code>model</code> option can specify one of the models returned
  by the <a href="#models">models</a> method.</li>

  <li>Version info in the <code>-YYMMDD</code> format can be left out when
  supplying <code>model</code> option &ndash; the latest avilable model will be
  used.</li>

  <li>The <code>model</code> option may be only several first words of model
  name (for example <code>czech</code>). In this case, the latest most suitable
  model is used.</li>
</ul>

<hr />

<h2 id="using_curl">Using Curl to Access the API</h2>

The described API can be comfortably used by <code>curl</code>. Several examples follow:

<h5>Passing Input on Command Line (if UTF-8 locale is being used)</h5>
<pre class="prettyprint lang-sh">curl --data-urlencode 'data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.' http://lindat.mff.cuni.cz/services/korektor/api/correct</pre>

<h5>Using Files as Input (files must be in UTF-8 encoding)</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' http://lindat.mff.cuni.cz/services/korektor/api/suggestions</pre>

<h5>Specifying Additional Parameters</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' -F 'model=czech-diacritics_generator' -F 'suggestions=3' http://lindat.mff.cuni.cz/services/korektor/api/suggestions</pre>

<h5>Converting JSON Result to Plain Text</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' http://lindat.mff.cuni.cz/services/korektor/api/correct | python -c "import sys,json; sys.stdout.write(json.load(sys.stdin)['result'])"</pre>

<?php require('footer.php') ?>
