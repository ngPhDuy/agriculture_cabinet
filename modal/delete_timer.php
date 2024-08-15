<?php
require_once 'connection_db.php';

$timer_id = $_GET['timer_id'];

if (isset($timer_id)) {
    $sql = "DELETE FROM timers WHERE id = '$timer_id'";
    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Timer deleted successfully";
            echo json_encode(array("message" => "Timer deleted successfully", "status" => 200));
        } else {
            throw new Exception("Error: " . $sql . "<br>" . $conn->error);
        }
    } catch (Exception $e) {
        http_response_code(400);
        $log = "Error: " . $e->getMessage();
        echo json_encode(array("message" => "Error: " . $e->getMessage(), "status" => 400));
    }
    $conn->close();
}

?>