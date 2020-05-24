<?php

$curl = curl_init();

curl_setopt_array($curl, array(
  CURLOPT_URL => "http://newsapi.org/v2/everything?q=indinero&from=2018-11-22&to=2018-12-21&sortBy=popularity&apiKey=d1e584a164314677a933b1cdd3e7e919",
  CURLOPT_RETURNTRANSFER => true,
  CURLOPT_TIMEOUT => 30,
  CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
  CURLOPT_CUSTOMREQUEST => "GET",
  CURLOPT_HTTPHEADER => array(
    "cache-control: no-cache"
  ),
));

$response = curl_exec($curl);
$err = curl_error($curl);

if ($err != "") {
    echo  "Curl Error: " . $err . "<p></p>";
    die();
}

curl_close($curl);

$response = json_decode($response, true); //because of true, it's in an array

$status = $response["status"];
if ($status == "error") {
    $err = $status = $response["message"];
    echo  "newsapi Error: " . $err . "<p></p>";
    die();
}


$results = $response["totalResults"];
echo "Found $results results! <br>";

for ($i=0;$i < $results; $i++) {

    $source = $response["articles"][$i]["source"]["name"];
    $title = $response["articles"][$i]["title"];
    $url = $response["articles"][$i]["url"];


    echo  $i . ") " . $source . " -- " . "<a href='$url'>" . $title . "</a>";
    echo "<br>";
}




?>
