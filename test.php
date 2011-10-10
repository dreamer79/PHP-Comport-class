<?php
error_reporting(0);

$com= new Comport('/dev/uart');//change this if needed...
$com->bauds= 57600;
$com->data= 8;
$com->stop= 1;
$com->parity= 'N';
$com->rts=1;
$com->openPort();

$com->writeData('Test for concatenated param as it broke C...'.chr(0x5a).chr(0x5a).chr(0x5a).'
');
$com->writeData(chr(0x1b).'Z');
print 'before read
';
$t= $com->readData(32);
print 'after read
';
$com->closePort();
print '----RESULT RECEIVED----
';
var_dump($t);
print "------------------------------------\n";
?>
