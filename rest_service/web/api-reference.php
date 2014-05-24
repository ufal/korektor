<?php $main_page=basename(__FILE__); require('header.php') ?>

<ul>
  <li><a href="#api_reference">API Reference</a></li>
  <li><a href="#using_curl">Using Curl to Access the API</a></li>
</ul>

<p>The web service is freely available for testing. Respect the
<a href="http://creativecommons.org/licenses/by-nc-sa/3.0/">CC BY-NC-SA</a>
licence of the models &ndash; <b>explicit written permission of the authors is
required for any commercial exploitation of the system</b>. If you run the
service, you agree that the data obtained during testing can be used for further
improvemenets of the system. All comments and reactions are welcomed.</p>

<h2><a id="api_reference"></a>API Reference</h2>

<p>The MorphoDiTa REST API can be accessed directly or via any other web
programming tools that support standard HTTP request methods and JSON for output
handling.</p>

<table border='1' class='table table-bordered'>
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

<p>Return the list of available models. The default model (used
when user supplies no model to a method call) is also returned.</p>

<h4>JSON Response</h4>

<p>The response object contains two fields <code>models</code> (containing array
of existing model names) and <code>default_model</code> (one of the models which
is used when no model is specified).

<h5>Example JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "models": [
  "czech-spellchecker"
 ,"czech-diacritics_generator"
 ,"strip_diacritics"
 ]
,"default_model": "czech-spellchecker"
}
</pre>


<h4>Browser Example</h4>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/models</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/models')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h3>Method <a id='correct'>correct</a></h3>

<p>Auto-correct the given text according to chosen model and return the corrected text as a string.</p>

<table border='1' class='table table-bordered'>
<tr align="left"><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr align="left"><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
<tr align="left"><td>model</td><td>no</td><td>string</td><td>The model to use. If not specified, <code>default_model</code> returned by <a href="#models">models</a> call is used.</td></tr>
</table>

<h4>JSON Response</h4>
<p>In case of failure, the response object contains a field <code>error</code>
with the error description. In case of success, the response object contains two
fields <code>model</code> (which model was used) and <code>result</code> 
(corrected text).

<h5>Example Success JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "model": "czech-spellchecker"
,"result": "Příliš žluťoučký kůň úpěl ďábelské ódy."
}
</pre>

<h5>Example Failure JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "error": "Specified model 'sstrip_diacritics' does not exist."
}
</pre>

<h4>Browser Examples</h4>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.')">try&nbsp;this</button></td></tr>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/correct?data=Příliš žluťoučký kůň úpěl ďábelské ódy.&amp;model=strip_diacritics</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/correct?data=Příliš žluťoučký kůň úpěl ďábelské ódy.&amp;model=strip_diacritics')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h3>Method <a id='suggestions'>suggestions</a></h3>

<p>Generate spelling suggestions for the given text. For every located error,
a list of suggestions is returned, from the most probable to the least probable.
User can specify the limit on number of suggestions returned.</p>

<table border='1' class='table table-bordered'>
<tr align="left"><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr align="left"><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
<tr align="left"><td>model</td><td>no</td><td>string</td><td>The model to use. If not specified, <code>default_model</code> returned by <a href="#models">models</a> call is used.</td></tr>
<tr align="left"><td>suggestions</td><td>no</td><td>positive integer</td><td>The maximum number of suggestions to return for a single error. If unspecified, value 5 is used.</td></tr>
</table>

<h4>JSON Response</h4>
<p>In case of failure, the response object contains a field <code>error</code>
with the error description. In case of success, the response object contains two
fields <code>model</code> (which model was used) and <code>result</code>,
which is an array of suggestions. Each suggestion is an array of strings, whose
first element is the original piece of text and the other elements (which may or
may not be present) are the suggestions, from the most probable to the least
probable. The concatenation of first elements of suggestions is equal to the
original text.

<h5>Example Success JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "model": "czech-spellchecker"
,"result": [
  [
   "Přílyš"
  ,"Příliš"
  ,"Přílis"
  ]
 ,[
   " "
  ]
 ,[
   "žluťoučky"
  ,"žluťoučký"
  ,"žluťoučké"
  ]
 ,[
   " kůň "
  ]
 ,[
   "ůpěl"
  ,"úpěl"
  ,"pěl"
  ]
 ,[
   " ďábelské "
  ]
 ,[
   "ódi"
  ,"ódy"
  ,"zdi"
  ]
 ,[
   "."
  ]
 ]
}
</pre>

<h5>Example Failure JSON Response</h5>
<pre class="prettyprint lang-json">
{
 "error": "Specified number of suggestions '0' is not a positive integer."
}
</pre>

<h4>Browser Examples</h4>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/suggestions?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/suggestions?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.')">try&nbsp;this</button></td></tr>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/suggestions?data=Prilis zlutoucky kun upel dabelske ody.&amp;model=czech-diacritics_generator&amp;suggestions=3</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/suggestions?data=Prilis zlutoucky kun upel dabelske ody.&amp;model=czech-diacritics_generator&amp;suggestions=3')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h2><a id="using_curl"></a>Using Curl to Access the API</h2>

The described API can be comfortably used by <code>curl</code>. Several examples follow:

<h5>Passing Input on Command Line (if UTF-8 locale is being used)</h5>
<pre class="prettyprint lang-sh">curl --data-urlencode 'data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.' http://quest.ms.mff.cuni.cz/korektor/api/correct</pre>

<h5>Using Files as Input (files must be in UTF-8 encoding)</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' http://quest.ms.mff.cuni.cz/korektor/api/suggestions</pre>

<h5>Specifying Additional Parameters</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' -F 'model=czech-diacritics_generator' -F 'suggestions=3' http://quest.ms.mff.cuni.cz/korektor/api/suggestions</pre>

<h5>Converting JSON Result to Plain Text</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' http://quest.ms.mff.cuni.cz/korektor/api/correct | python -c "import sys,json; sys.stdout.write(json.load(sys.stdin)['result'])"</pre>

<?php require('footer.php') ?>
