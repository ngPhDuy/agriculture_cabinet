<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$data = json_decode(file_get_contents('php://input'), true);
$device_name = $data['device_name'];
$device_description = $data['device_description'];
$relay_id = $data['relay_id'];
if (isset($device_name) && isset($relay_id)) {
    echo "Post handler";

    echo $device_name;

    $sql = "INSERT INTO devices (id, name, description, state) VALUES ('$relay_id', '$device_name', '$device_description', 0)";

    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Device added successfully";
            echo json_encode(array("message" => "Device added successfully", "status" => 200));
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