<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$data = json_decode(file_get_contents('php://input'), true);

$timer_id = $data['timer_id'];
$state = $data['state'];

if (isset($timer_id) && isset($state)) {

    $sql = "UPDATE timers SET state = '$state' WHERE id = '$timer_id'";

    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Timer updated successfully";
            echo json_encode(array("message" => "Timer updated successfully", "status" => 200));
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