<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$data = json_decode(file_get_contents('php://input'), true);

$relay_id = $data['relay_id'];
$state = $data['state'];

if (isset($relay_id) && isset($state)) {

    $sql = "UPDATE devices SET state = '$state' WHERE id = '$relay_id'";

    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Device updated successfully";
            echo json_encode(array("message" => "Device updated successfully", "status" => 200));
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
?>