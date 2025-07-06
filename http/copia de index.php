<?php
 // Append the query string if it exists and isn't null
 if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
    $argument = $_SERVER['QUERY_STRING'];
    echo `perl index.htm $argument`;
 } else {
    echo `perl index.htm`;
}
?>
