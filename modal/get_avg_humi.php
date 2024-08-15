<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$today = date("Y-m-d");
$sql = "call get_humidity_per_hour('$today')";
$avg_temps = [];
try {
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        while ($row = $result->fetch_assoc()) {
            $avg_temps[] = $row;
        }
    }
    header('Content-Type: application/json');
    http_response_code(200);
    echo json_encode($avg_temps);
} catch (Exception $e) {
    http_response_code(400);
    echo json_encode(array("message" => "Error: " . $e->getMessage(), "status" => 400));
}
?>