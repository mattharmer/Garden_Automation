<?php

$servername = "localhost";

// REPLACE with your Database name
$dbname = "mattharmer_garden";
// REPLACE with Database user
$username = "mattharmer_hiveadmin";
// REPLACE with Database user password
$password = "y^=9B!j3b(5w";

// Keep this API Key value to be compatible with the ESP32 code provided in the project page. If you change this value, the ESP32 sketch needs to match
$api_key_value = "tPmAT5Ab3j7F9";

$api_key = $valueDHT11t = $valueDHT11h = $valueTZT  = $valueCSMSV2 = $valueDS18B20 = $valueRELAY01 = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $valueDHT11t = test_input($_POST["valueDHT11t"]);
        $valueDHT11h = test_input($_POST["valueDHT11h"]);
        $valueTZT = test_input($_POST["valueTZT"]);
        $valueCSMSV2 = test_input($_POST["valueCSMSV2"]);
        $valueDS18B20 = test_input($_POST["valueDS18B20"]);
        $valueRELAY01 = test_input($_POST["valueRELAY01"]);


        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO Sensor (valueDHT11t, valueDHT11h, valueTZT, valueCSMSV2, valueDS18B20, valueRELAY01)
        VALUES ('" . $valueDHT11t . "', '" . $valueDHT11h . "', '" . $valueTZT . "', '" . $valueCSMSV2 . "', '" . $valueDS18B20 . "', '" . $valueRELAY01 . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
