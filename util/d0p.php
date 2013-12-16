<?php // d0p structure, php-5.5.3

class d0p {

	private $outputHandle=null;
	private $inputHandle=null;

	# void, ctor
	public function __construct($output, $mode='w') {
		if(is_string($output)) {
			$this->outputHandle=fopen($output,$mode);
		} elseif(is_resource($output)) {
			$this->outputHandle=$output;
		} else throw new BadMethodCallException("No output specified");
	}
	
	# void, dtor
	public function __destruct() {
		!is_null($this->outputHandle) && fclose($this->outputHandle);
		!is_null($this->inputHandle) && fclose($this->inputHandle);
	}
	
	# void
	public function setInput($input, $mode='r') {
		if(is_string($input)) {
			if(!file_exists($input)) touch($input);
			$this->inputHandle=fopen($input,$mode);
		} elseif(is_resource($output)) {
			$this->inputHandle=$output;
		} else throw new BadMethodCallException("No input specified");
	}

	# bool
	public function wrap($object) {
		// prepair object
		$object_dc = serialize($object);
		
		// generate a key and the 16b sequence
		$key = uniqid();
		echo "-> Key: $key\n";

		// now, create the whole sequence.
		$d0p = implode("", array(
			"$key\0\0\0", 					#key
			convert_uuencode($object_dc),	#info
			"\0\0\0$key" 					#key
			  								#data, add with loop
		));
		fwrite($this->outputHandle, $d0p);
		
		// input to output
		$sz = fsize($this->inputHandle);
		echo "-> About to write $sz byzes";
		while($junk = fread($this->inputHandle, 1024)) {
			$b = ftell($this->inputHandle);
			echo "\r{".$b."/".$sz."} Writing...";
			fwrite($this->outputHandle, $junk);
		}
		echo " - Done\n";
	}

	# object
	public function unwrap() {
		// reverse operation....
		// 1: Read 0-13th byte
		$key = fread($this->outputHandle, 13);
		echo "-> Key: $key\n";
		echo "-> Current byte: ".ftell($this->outputHandle)."\n";
		
		// shift over the tripple null-byte sequence
		fseek($this->outputHandle, 16);
		
		// 2: Read untill the key re-appears. That shall be done by reading 16/sequence
		$seq="\0\0\0$key";
		$header=null;
		$fsize = fsize($this->outputHandle);
		echo "{".ftell($this->outputHandle)."/".$fsize."} Now reading...";
		while($read = fread($this->outputHandle, 3)) { // Looooook for the nullbytes.
			echo "\r{".ftell($this->outputHandle)."/".$fsize."} Reading header...";
			if($read == "\0\0\0") {
				echo "Done. \n";
				fread($this->outputHandle, 13); #skip key
				break;
			} else {
				$header .= $read;
			}
		}
		
		// 3: Header now contains the full, decoded, header. Extract it.
		$header_dc = (object)unserialize(convert_uudecode($header));
		
		// 4: Write the rest of the archive to disk. Herefore, we should utilize the real filename.
		$info = stream_get_meta_data($this->outputHandle);
		if(!is_resource($this->inputHandle)) return false;
		while(!feof($this->outputHandle)) {
			echo "\r{".ftell($this->outputHandle)."/".$fsize."} Writing archive ...";
			fwrite($this->inputHandle, fread($this->outputHandle, 1024));
		}
		echo " - Done.\n";
		
		// Add the freshly created filename.
		$header_dc->archive=$info['uri'].".tar.gz";
		
		return $header_dc;
	}

}

function fsize($res) {
	!is_resource($res) && trigger_error("Argument 1 was expected to be resource; ".gettype($res)." given.");
	$curByte = ftell($res);
	$nowByte = null;
	while(!feof($res)) {
		fread($res, 1024);
		$nowByte = ftell($res);
	}
	fseek($res, $curByte);
	return $nowByte;
}

// random bytes
$random_fh = fopen("/dev/random","r");
$randomBytes = fread($random_fh, 512);
fclose($random_fh);

$d0p = new d0p("test.d0p",'r');
$d0p->setInput("_new.tgz",'w');
print_r($d0p->unwrap());
