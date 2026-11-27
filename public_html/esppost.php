<?php
 $Light=$_POST["light"]; // Light Variable
 $Write="<p>Light Level: " . $Light . " % </p>"; // Create HTML file to put data in.
 file_put_contents('sensor.html',$Write);
?>
