DROP DATABASE IF EXISTS toysvm;
CREATE DATABASE toysvm;

use toysvm;

CREATE TABLE toy (one int, x double, y double, z double, lbl boolean);

INSERT INTO toy VALUES (1, 5.0, 5.0, 1.0, true);
INSERT INTO toy VALUES (1, 3.0, 3.0, 0.5, false);
INSERT INTO toy VALUES (1, 10.0, 10.0, 2.5, true);
INSERT INTO toy VALUES (1, 1.0, 2.5, 0.6, false);
