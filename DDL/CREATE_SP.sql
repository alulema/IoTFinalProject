DROP PROCEDURE GetDataSamples;

DELIMITER //
CREATE PROCEDURE GetDataSamples (IN rate INT, IN devId VARCHAR(16))
BEGIN
  SET @spacer := 1;
  SET @count := (SELECT COUNT(*) from iot_projectw6.Temperature WHERE timestamp > DATE_SUB(NOW(), INTERVAL rate MINUTE) AND device_id = devId ORDER BY timestamp DESC);
  SET @spacer:= IF(@count > 100, ROUND(@count / 60), @spacer);

  IF @spacer = 1 THEN
    SELECT * FROM iot_projectw6.Temperature AS Tp WHERE Tp.timestamp > DATE_SUB(NOW(), INTERVAL rate MINUTE ) AND device_id = devId ORDER BY Tp.timestamp DESC;
  ELSE
    SELECT id, device_id, device_status, measurement, unit, timestamp
    FROM (
        SELECT
            @row := @row +1 AS rownum, Tp.*
        FROM (
            SELECT @row :=0) r, iot_projectw6.Temperature AS Tp WHERE Tp.timestamp > DATE_SUB(NOW(), INTERVAL rate MINUTE) AND device_id = devId ORDER BY Tp.timestamp DESC
        ) ranked
    WHERE rownum % @spacer = 0;
  END IF;
END //
DELIMITER ;

# example
CALL GetDataSamples(1, 'THE-001');
