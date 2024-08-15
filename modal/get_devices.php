<?php
require_once 'connection_db.php';

$query = "SELECT * FROM devices";
$result = $conn->query($query);

$devices = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $devices[] = $row;
    }
}

header('Content-Type: application/json');
echo json_encode($devices);

$conn->close();
?>