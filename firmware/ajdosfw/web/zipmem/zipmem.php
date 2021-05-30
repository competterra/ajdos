<?php
/*
 * zipmem.php 
 * v1.0
 * 
 * Convert a file to a C++ header file, contains the original data from the file in binary array source code.
 * Enter "php zipmem.php" without prameters for help.
 * 
 * v1.0 - 2020.11.15 
 * 
 * Todo:
 * - Add js/css/html minimize before use gzip
 *
 * Copyright (c) 2017 Compet-Terra ltd.
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *           
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 * 
*/
if (php_sapi_name() !== "cli" || ! isset($argv) )
    die;

$aParams = array();
foreach ( $argv as $n => $arg )
{
    $aParams[] = $arg;
}       

switch ( $aParams[1] )
{
    case "pack":
    case "-p":  
        if ( isset($aParams[2]) && isset($aParams[3]) )
        {
            if ( is_file($aParams[2])  )  
            {
                if ( false !== ($gzName = makegz( $aParams[2])) )
                {
                    zm_pack( $gzName, $aParams[3] );
                    return;
                }
                else
                    echo "ERROR: gzip not created {$aParams[2]}.gz\r\n";
            }
            else
                echo "ERROR: invalid filename {$aParams[2]}\r\n";
        }
        else
        {
            echo "ERROR: to few parameters\r\n";
            help();
        }
        break;

    case "exrtact":        
    case "-e":
        if ( isset($aParams[2]) )
        {
            if ( is_file($aParams[2])  )  
            {
                zm_extract( $aParams[2] );
                return;
            }
            else
                echo "ERROR: invalid filename {$aParams[2]}\r\n";
        }
        else
        {
            echo "ERROR: to few parameters\r\n";
            help();
        }
        break;

    default:
        echo "ERROR: Invalid parameters.\r\n";
        help();
    die;    
}

function help()
{
    echo "Usage:\r\n";
    echo "zipmem { [pack | -p] | [extract | -e] } {filename} [varname]\r\n\r\n";
    echo "pack | -p\tpacking a file to a C header\r\n";
    echo "extract | -e\textract a file from a C header\r\n";
    echo "{filename}\tfile name\r\n";
    echo "\t\t- the original filename when packing\r\n";
    echo "\t\t- the filename of the C header file\r\n";
    echo "varname\t\tname of the variable while packing";
}


function zm_extract( $filename )
{
    $pi     = pathinfo($filename);
    $output = $pi['filename'].".zip";

    $handle = @fopen("$filename", "r");
    $line   = 0;
    $binbuf = "";
    while (($buffer = fgets($handle, 4096)) !== false) 
    {
        +$line;
        if ( substr($buffer,0,2) == "0x" )
        {
           $aData = explode(",", $buffer );
           foreach( $aData as $hex )
           {
                if ( substr(trim($hex),0,2) == "0x" )
                {
                    $binbuf .= hex2bin( substr(trim($hex),2,2) );
                }
           }
        }
    }
    fclose($handle);
    file_put_contents( $output, $binbuf );
}


function zm_pack( $filename, $varname )
{
    $pi     = pathinfo($filename);
    $output = $pi['filename'].".h";

    $handle = @fopen("$filename", "r");
    $binbuf = fread( $handle, ($fs = filesize( $filename )));
    $outbuf = "#define {$varname}_len $fs\r\n";
    $outbuf.= "const uint8_t {$varname}[] PROGMEM = {\r\n";
    $n=0;    
    for ( $i=0; $i<$fs; ++$i)
    {
        $outbuf .= "0x".strtoupper(bin2hex( substr($binbuf, $i, 1 )) ).", ";
        if ( ++$n % 16 == 0 )
            $outbuf .= "\r\n";
    }
    $outbuf.= "};";
    file_put_contents( $output, $outbuf );
    fclose($handle);
}

function makegz( $filename, $level=9 )
{
    $bFinish = false;
    if ($fp_in = fopen( $filename,'rb')) 
    { 
        mkdir("./temp");
        $gzName = './temp/' . $filename . '.gz';
        if ( $fp_out = gzopen( $gzName, 'wb' . $level) ) 
        {        
            while (!feof($fp_in)) 
                gzwrite($fp_out, fread($fp_in, 1024 * 512)); 
            
            gzclose($fp_out); 
            $bFinish = $gzName;
        }
        fclose($fp_in); 
    }
    return $bFinish;
}

?>