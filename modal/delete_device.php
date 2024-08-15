<?php
require_once 'connection_db.php';

$relay_id = $_GET['relay_id'];
if (isset($relay_id)) {
    $sql = "DELETE FROM devices WHERE id = '$relay_id'";
    try {
        if ($conn->query($sql) === TRUE) {
            http_response_code(200);
            $log = "Device deleted successfully";
            echo json_encode(array("message" => "Device deleted successfully", "status" => 200));
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