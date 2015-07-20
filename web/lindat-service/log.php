<?php

# Use CORS.
header("Access-Control-Allow-Origin: *");

# Do we have required arguments?
if (array_key_exists('original', $_POST) &&
    array_key_exists('korektor', $_POST) &&
    array_key_exists('corrected', $_POST)) {
  # Format log.
  $line = trim(strtr($_POST["original"], "\n\t", "\r ")) . "\t" . trim(strtr($_POST["korektor"], "\n\t", "\r ")) . "\t" . trim(strtr($_POST["corrected"], "\n\t", "\r ")) . (array_key_exists('origin', $_POST) ? "\t" . trim(strtr($_POST["origin"], "\n\t", "\r ")) : "") . "\n";

  # Try appending it.
  if (strlen($line) > 3 && file_put_contents("/home/straka/korektor-logs/corrections.txt", date(DATE_RFC2822) . "\t" . $line, FILE_APPEND | LOCK_EX) !== false)
    echo "{\"success\": \"true\"}\n";
}

?>
