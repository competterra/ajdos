<?php
    // setup the URL and read the JS from a file
    $url = 'https://javascript-minifier.com/raw';
    $js = file_get_contents('../ajdos-setup.js');
    // init the request, set various options, and send it
    $ch = curl_init();

    curl_setopt_array($ch, [
        CURLOPT_SSL_VERIFYHOST => false,
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_URL => $url,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_POST => true,
        CURLOPT_HTTPHEADER => ["Content-Type: application/x-www-form-urlencoded"],
        CURLOPT_POSTFIELDS => http_build_query([ "input" => $js ])
    ]);
    
    $minified = curl_exec($ch);
    var_dump($minified);
    if ( !$minified )
    {
        trigger_error(curl_error($ch));
    }
    // finally, close the request
    curl_close($ch);

    // output the $minified JavaScript
    echo $minified;
?>