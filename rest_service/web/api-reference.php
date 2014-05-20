<?php $main_page=basename(__FILE__); require('header.php') ?>

<ul>
  <li><a href="#api_reference">API Reference</a></li>
  <li><a href="#response_format">Common Response Format</a></li>
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
    <td><a href="#auto_correct">auto_correct</a></td>
    <td>perform autocorrection of the given text</td>
    <td>GET/POST</td>
</tr>
<tr>
    <td><a href="#generate_diacritics">generate_diacritics</a></td>
    <td>generate diacritics of the given text</td>
    <td>GET/POST</td>
</tr>
<tr>
    <td><a href="#strip_diacritics">strip_diacritics</a></td>
    <td>strip diacritics of the given text</td>
    <td>GET/POST</td>
</tr>
</table>

<h3>Method <a id='auto_correct'>auto_correct</a></h3>

<p>Auto-correct the given text. The output format is
 <a href="#response_format">described later</a>.</p>

<table border='1' class='table table-bordered'>
<tr align="left"><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr align="left"><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
</table>

<h5>Browser Examples</h5>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/auto_correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/auto_correct?data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h3>Method <a id='generate_diacritics'>generate_diacritics</a></h3>

<p>Generate diacritics of the given text. Works similarly to auto_correct, but
only changes in diacritics are performed. The output format is
<a href="#response_format">described later</a>.</p>

<table border='1' class='table table-bordered'>
<tr align="left"><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr align="left"><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
</table>

<h5>Browser Examples</h5>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/generate_diacritics?data=Prilis zlutoucky kun upel dabelske ody.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/generate_diacritics?data=Prilis zlutoucky kun upel dabelske ody.')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h3>Method <a id='strip_diacritics'>strip_diacritics</a></h3>

<p>Strip diacritics of the given text. The output format is
<a href="#response_format">described later</a>.</p>

<table border='1' class='table table-bordered'>
<tr align="left"><th>Parameter</th><th>Mandatory</th><th>Data type</th><th>Description</th></tr>
<tr align="left"><td>data</td><td>yes</td><td>string</td><td>Input text in <b>UTF-8</b>.</td></tr>
</table>

<h5>Browser Examples</h5>
<table style='width: 100%'>
 <tr><td style='vertical-align: middle'><pre style='margin-bottom: 0' class="prettyprint lang-html">http://quest.ms.mff.cuni.cz/korektor/api/strip_diacritics?data=Příliš žluťoučký kůň úpěl ďábelské ódy.</pre></td>
     <td style='vertical-align: middle'><button style='width: 100%' type="button" class="btn btn-success btn-xs" onclick="window.open('http://quest.ms.mff.cuni.cz/korektor/api/strip_diacritics?data=Příliš žluťoučký kůň úpěl ďábelské ódy.')">try&nbsp;this</button></td></tr>
</table>

<hr />

<h2><a id="response_format"></a>Common Response Format</h2>

<p>The response format of all methods is
<a href="http://en.wikipedia.org/wiki/JSON">JSON</a> in the following format:</p>

<h3>Success</h3>
<!--<pre class="prettyprint lang-json">
{
 "model": "Model used"
,"acknowledgements": ["URL with acknowledgements", ...]
,"result": [ suggestion, suggestion, ... ]
}
</pre>-->
<pre class="prettyprint lang-json">
{
 "result": [ suggestion, suggestion, ... ]
}
</pre>

where <code>suggestion</code> is an array of strings, the first is the original
piece of text and the others (which may or may not be present) are the
suggestions, from the most probable to the least probable.

<h3>Failure</h3>
<pre class="prettyprint lang-json">
{
 "error": "Description of the error"
}
</pre>

<hr />

<h2><a id="using_curl"></a>Using Curl to Access the API</h2>

The described API can be comfortably used by <code>curl</code>. Several examples follow:

<h5>Passing Input on Command Line (if UTF-8 locale is being used)</h5>
<pre class="prettyprint lang-sh">curl --data-urlencode 'data=Přílyš žluťoučky kůň ůpěl ďábelské ódi.' http://quest.ms.mff.cuni.cz/korektor/api/auto_correct</pre>

<h5>Using Files as Input (files must be in UTF-8 encoding)</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' http://quest.ms.mff.cuni.cz/korektor/api/auto_correct</pre>

<!--<h5>Specifying Additional Parameters</h5>
<pre class="prettyprint lang-sh">curl -F 'data=@input_file' -F 'output=vertical' http://quest.ms.mff.cuni.cz/korektor/api/auto_correct</pre>-->

<?php require('footer.php') ?>
