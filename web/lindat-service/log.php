<?php

$line = strtr($_POST["original"], "\n\t", "  ") . "\t" . strtr($_POST["korektor"], "\n\t", "  ") . "\t" . strtr($_POST["corrected"], "\n\t", "  ") . "\n";
if (strlen($line) > 3 && file_put_contents("/home/straka/korektor-logs/corrections.txt", date(DATE_RFC2822) . "\t" . $line, FILE_APPEND | LOCK_EX) !== false)
  echo '{"success": "true"}';

?>
