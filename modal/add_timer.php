<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$data = json_decode(file_get_contents('php://input'), true);
$timer_name = $data['timer_name'];
$device_id = $data['device_id'];
$start_time = $data['start_time'];
$duration = $data['duration'];
if (isset($timer_name) && isset($device_id) && isset($start_time) && isset($duration)) {

    $sql = "INSERT INTO timers (name, device_id, start_time, duration, state) VALUES ('$timer_name', '$device_id', '$start_time', '$duration', 0)";

    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Timer added successfully";
            //get id of new timer
            $sql = "SELECT id FROM timers ORDER BY id DESC LIMIT 1";
            $result = $conn->query($sql);
            $row = $result->fetch_assoc();
            $timer_id = $row['id'];
            $log .= " with id: " . $timer_id;
            echo json_encode(array("message" => "Timer added successfully", "status" => 200, "id" => $timer_id));
        } else {
            throw new Exception("Error: " . $sql . "<br>" . $conn->error);
        }
    } catch (Exception $e) {
        http_response_code(400);
        $log = "Error: " . $e->getMessage();
        echo json_encode(array("message" => "Error: " . $e->getMessage(), "status" => 400));
    }

    $conn->close(); // Đóng kết nối cơ sở dữ liệu
}
file_put_contents('debug.log', $log, FILE_APPEND);
?>