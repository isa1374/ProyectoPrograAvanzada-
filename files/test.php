<?php
    echo "Hello, I'm a Test";
    $x=getenv('QUERY_STRING');
    $arr=$x.split("&");
    echo $arr;
?>
