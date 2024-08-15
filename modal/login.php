<?php
require_once 'connection_db.php';

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");
header('Content-Type: application/json');

$data = json_decode(file_get_contents('php://input'), true);
$username = $data['username'];
$password = $data['password'];
if (isset($username) && isset($password)) {

    $sql = "SELECT * FROM users WHERE username = '$username' AND password = '$password'";

    try {
        $result = $conn->query($sql);
        if ($result->num_rows <= 0) {
            http_response_code(400);
            $log = "Fail to login";
            echo json_encode(array("message" => "Fail to login", "status" => 400));
            return;
        }
        http_response_code(200);
        $log = "Login successfully";
        echo json_encode(array("message" => "Login successfully", "status" => 200));
    } catch (Exception $e) {
        http_response_code(400);
        $log = "Error: " . $e->getMessage();
        echo json_encode(array("message" => "Error: " . $e->getMessage(), "status" => 400));
    }

    $conn->close(); // Đóng kết nối cơ sở dữ liệu
}
file_put_contents('debug.log', $log, FILE_APPEND);


?>