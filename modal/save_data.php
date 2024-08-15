<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

date_default_timezone_set('Asia/Ho_Chi_Minh');
$data = json_decode(file_get_contents('php://input'), true);
$today = date("Y-m-d H:i:s");
$humidity = $data['humidity'];
$temperature = $data['temperature'];

$sql = "insert into temperatures (time_stamp, value) values ('$today', $temperature)";
try {
    if ($conn->query($sql) === TRUE) {
        $log = "New temperature record created successfully";
        $sql = "insert into humidities (time_stamp, value) values ('$today', $humidity)";
        if ($conn->query($sql) === TRUE) {
            $log = "New humidity record created successfully";
            http_response_code(200);
            echo json_encode(array("statusCode" => 200));
        } else {
            $log = "Error: " . $sql . "<br>" . $conn->error;
            http_response_code(201);
            echo json_encode(array("statusCode" => 201));
        }
    } else {
        $log = "Error: " . $sql . "<br>" . $conn->error;
        http_response_code(400);
        echo json_encode(array("statusCode" => 400));
    }
} catch (Exception $e) {
    $log = "Error: " . $e->getMessage();
    http_response_code(400);
    echo json_encode(array("statusCode" => 400));
}
file_put_contents('debug.log', $log, FILE_APPEND);
?>