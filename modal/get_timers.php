<?php
require_once 'connection_db.php';

$query = "select timers.*, devices.name as device_name, devices.id as device_id from timers join devices on timers.device_id = devices.id";
$result = $conn->query($query);

if ($result->num_rows > 0) {
    $timers = [];
    while ($row = $result->fetch_assoc()) {
        $timers[] = $row;
    }
}

header('Content-Type: application/json');
echo json_encode($timers);

$conn->close();
?>