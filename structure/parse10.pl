#!/usr/bin/perl
use File::Basename;

if (not $ARGV[0]) {
    print "Please specify target file.\n";
    exit 1;
}

# process arguments...
foreach $arg (@ARGV){
    if ($arg =~ m/^--include/g){
        ($a,$b)=split /=/,$arg,2;
        if (not -d $b){
            print "$b is not a directory\n";
        } else {
            $includePath = $b;
        }
    }
    elsif ($arg =~ m/^--templates/g){
        ($a,$b)=split /=/,$arg,2;
        if (not -d $b){
            print "$b is not a directory\n";
        } else {
            $templatePath = $b;
        }
    }
    elsif ($arg =~ m/^--problemTypeTag/g){
        ($a,$b)=split /=/,$arg,2;
        if (not -d $b){
            print "$b is not a directory\n";
        } else {
            $templatePath = $b;
        }
    }
    elsif (-e $arg) {
        $startFile = $arg;
        if (not -e $startFile){
            print "$startFile does not exist\n";
            exit 1;
        }
        print "<!-- Parsing $startFile -->\n";
    } 
}

if ($includePath){
    print "<!-- Additional include directory at $includePath -->\n";
} else {
    print "<!-- Additional include directory not specified -->\n";
}
if ($templatePath){
    print "<!-- Installed templates at $templatePath -->\n";
} else {
    print "<!-- Installed template location not specified -->\n";
    print "<!-- Will try to determine location from $startFile -->\n";
    $templatePath = &getInstallationPath($startFile);
}
if ($problemTypeTag){
    print "<!-- ProblemTypeTag manually specified to $problemTypeTag -->\n";
} else {
    print "<!-- Will try to determine ProblemTypeTag from source files -->\n";
    &getProblemTypeTag($startFile);
}


#$templatePath = &getInstallationPath($ARGV[0]);
# Dumux installation directory may be specified directly:
#$templatePath= "/home/edscott/GIT/Beck2019a/beck2019a_decoupled";

#extra includes not in dumux installation path nor source directory:
#$includePath= "/home/edscott/tmp";

@files;
%files;
$filecount=0;
%includes;


$currentDir = `pwd`; chop $currentDir;

&openFileRecurse($startFile);
@reversed = reverse @files;
@files = @reversed;
foreach $f (@files){ print "$f\n"; }
print "HALT\n"; exit 1;
@typeTags;
%typeTagFiles;
%typeTagLineNumber;
%properties;
%propertyFiles;
%propertyValues;
%propertyTypeTag;
%propertySource;
%propertyLineNumber;
%inherits;
%focus;
$referenceLineCount;

$debug=0;
$verbose=0;
$count = 0;
if ($debug){print("TYPE_TAGS:\n")}
foreach $f (@files){&getTypeTags($f)}
foreach $f (@files){&getTypeInherits($f)}
&printInherits;
$count = 0;
foreach $f (@files){&getProperties($f)}
if ($verbose) {&printProperties}
foreach $f (@files){&getPropertyValues($f)}
if (defined $problemTypeTag){
    &markFocus($problemTypeTag, 1);
}
open OUTPUT, ">./structure.xml" or die "cannot open structure.xml";
&writeXML;
close OUTPUT;

exit 1;

sub getProblemTypeTag {
    undef $problemTypeTag;
    my($inFile) = @_;
    my $line=1;
    open IN, "$inFile" or die "getProblemTypeTag:: Cannot open $inFile for processing.\n";
    while (<IN>){
        s/^\s+//;
        if (/^\/\//){next}
        if (/typedef/ and /TTAG/ and /TypeTag/){
            ($a,$b) = split /\(/, $_, 2;
            ($problemTypeTag,$c) = split /\)/, $b, 2;
            print "<!-- Problem TypeTag = \"$problemTypeTag\" from file $inFile line $line-->\n";
#chop; print "<!-- $_ -->\n";
            break;
        }
        $line++;
    }
    close IN;
    if (not $problemTypeTag){
        print "<!-- *** Warning: Dumux::Problem TypeTag not found-->\n";
    }

}

# Find Dumux installation path from current file...
sub getInstallationPath {
    my ($infile) = @_;
    my $currentDir = dirname($infile);
    my $realpath = &realpath($currentDir);
    my $test = $realpath . "/dumux";
    if (-d $test) {
        print "<!-- Assuming installation templates at \"$test\" -->\n";
        return $realpath
    }
    if ($realpath eq "/") {
        print "<!-- *** Warning: Dumux installation templates not found-->\n";
        return $realpath
    }
    return &getInstallationPath($realpath);
}

sub markFocus {
    my ($focus, $focusLevel) = @_;
    if (not $focus{$focus}){$focus{$focus} = $focusLevel}

    if ($debug){print "focus: $focus\n";}
    my @array = @{ $inherits{$focus} };
    my $subfocus;
    foreach $subfocus (@array){
        &markFocus($subfocus, $focusLevel+1);
    }
}


sub printFilesXML {
    my ($sFile, $level) = @_;
    my @keys;
    my $file;
    my @array;
    my $i;
    for ($i=0; $i<$level; $i++) {print OUTPUT " "}
    $oFile = $sFile;
    $oFile =~ s/$templatePath\///;
    if ($includePath) {$oFile =~ s/$includePath\///;}
    my $realpath = `realpath $sFile`;
    chop $realpath;
    print OUTPUT "<files name=\"$oFile\" realpath=\"$realpath\">\n";
    my @array = @{ $files{$sFile} };
    foreach $file (@array){
        &printFilesXML($file, $level+1);
    }
    for ($i=0; $i<$level; $i++) {print OUTPUT " "} 
    print OUTPUT "</files>\n";
}

sub openFile {
    my ($path) = @_;
#print "parsing $path ...\n";
    
    my $stream;
    $pwd =`pwd`; chop $pwd;
    open $stream, $path or die "cannot open $path (pwd=$pwd)\n";
    my  $a, $b, $c, $d;
    $comment = 0;
    while (<$stream>){
        s/^\s+//;
        if (/^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (not /^#/) {next}
        if (not /#include/) {next}
        if (/"/ or (/</ and />/)) {
            if (/</ and />/)  {
                ($a, $c) = split /</, $_, 2;
#                print "a c = $a $c\n";
                ($b, $a) = split />/, $c, 2;
#                print "b a  = $b $a\n";
                $b =~ s/^\s+//;
            } else {
#               relative includes...
                ($a, $b, $c) = split /"/, $_, 3;
#                print "relative includes... a b c = $a $b $c\n";
                $b=~ s/^\s+//;
            }

            $dirname = dirname($path);
            $nextFile = $dirname . "/" . $b;

#           1. if in current or relative directory, use it.
	    if (-e $nextFile and /"/) {
                push(@{ $files{$path} }, $nextFile);
                &openFileRecurse($nextFile);          
                next;
	    }
#           2. try include path (user templates)
            if ($includePath) {
                $nextFile = $includePath . "/" . $b;
	        if (-e $nextFile) {
                    push(@{ $files{$path} }, $nextFile);
                    &openFileRecurse($nextFile);          
                    next;
                }
            }
#           3. try templates path (installation templates)
            if ($templatePath) {
                $nextFile = $templatePath . "/" . $b;
	        if (-e $nextFile) {
                    push(@{ $files{$path} }, $nextFile);
                    &openFileRecurse($nextFile);          
                    next;
                }
            }
#           4. skip system includes
            $nextFile = "/usr/include/" . $b;  
            if (-e $nextFile) {
                print "<!-- skipping header $nextFile -->\n";
                next;
            }  
            if ($b =~ m/^dune/g) {next} # ignore dune 
            if (not $b =~ m/\.h/g) {next} # ignore stdc++
            print "**** SKIPPING  <$b> referenced in $path\n";          
            
        }
       

    }
    close $stream;
}


sub openFileRecurse {
    my ($path) = @_;
    if ($fileList{$path}){
#                print "*** $path already included...\n";
    } else {
#               print "*** $path --> $d\n";
        $fileList{$path} = 1;
        push(@files, $path);
        &openFile($path);    
    } 
}

sub writeXML {
    my $extraIncludes = "";
    if ($includePath) {
        $extraIncludes = " include=\"$includePath\"";
    } 
    print OUTPUT <<EOF;
<?xml version="1.0"?>
<dumux-info xmlns:xffm="http://www.imp.mx/">
<structure source=\"$ARGV[0]\" templates=\"$templatePath/dumux\" $extraIncludes/>
EOF
&printFilesXML($startFile, 0);
&printPropertiesXML;
&printTypeTagsXML;
print OUTPUT "</dumux-info>\n";
}

sub realpath{
    my ($inPath) = @_;
    if (not -e $inPath){
        if (-e $templatePath."/$inPath"){ $inPath = $templatePath."/$inPath"}
        elsif ($includePath){
            if (-e $includePath."/$inPath"){ $inPath = $includePath."/$inPath"}
            else {return $inPath}
        }
    } 
    my $realpath = `realpath $inPath`;
    chop $realpath;
    return $realpath;
}

sub printTypeTagsXML{
    my %repeat;
    my $typetag;
#    my @keys = sort @typeTags;
    my @keys = sort {$focus{$a} <=> $focus{$b}} @typeTags;
    foreach $typetag (@keys){
        if ($repeat{$typetag}) {
            print OUTPUT "<!-- Repeated typetag name: $typetag -->\n";
            next;
        }
        $repeat{$typetag} = 1;
        my $inheritString = &getInheritString($typetag);

        my $oFile = $typeTagFiles{$typetag};
        $oFile =~ s/$templatePath\///;
        if ($includePath) {$oFile =~ s/$includePath\///;}
        if ($focus{$typetag}){ $focusItem = " focus=\"$focus{$typetag}\""}
        else {undef $focusItem;}
        my $realpath = &realpath($oFile);
        if ($focusItem){
            my $spaces = "";
            my $i;
            for ($i=0; $i<$focus{$typetag}; $i++){
                $spaces .= "   ";
            }
            print OUTPUT " <typetag name=\"$spaces$typetag\" inherits=\"$inheritString\" source=\"$oFile:$typeTagLineNumber{$typetag}\" realpath=\"$realpath\" $focusItem>\n";
            my @akeys = keys(%propertyTypeTag);
            my @skeys = sort @akeys;
            my $property;
            foreach $property (@skeys){
                #property info here
                if ($propertyTypeTag{$property} eq $typetag) {
                    my $value = $propertyValues{$property};
                    $value =~ s/</&lt;/g;
                    $value =~ s/>/&gt;/g;
                    $oFile = $propertySource{$property};
                    $oFile =~ s/$templatePath\///;
                    if ($includePath){$oFile =~ s/$includePath\///;}
                    $realpath = &realpath($oFile);
                    print OUTPUT "  <property name=\"$property\" value=\"$propertyValues{$property}\" source=\"$oFile:$propertyLineNumber{$property}\" realpath=\"$realpath\"/>\n";
                }
            }
            print OUTPUT " </typetag>\n";
        }
    }

}

sub printPropertiesXML{
    my $property;
    my @keys = keys (%properties);
    my @properties = sort @keys;
    foreach $property (@properties){
        $typetag = $propertyTypeTag{$property};
        $source = $propertySource{$property};
        my $lineNumber = $propertyLineNumber{$property};
        $oFile = $source;
        $oFile =~ s/$templatePath\///;
        if ($includePath){$oFile =~ s/$includePath\///;}
        $value = $propertyValues{$property};
        $value =~ s/</&lt;/g;
        $value =~ s/>/&gt;/g;
        my $realpath = &realpath($oFile);
        if ($value ne "undefined"){
            print OUTPUT " <property name=\"$property\" typetag=\"$typetag\" value=\"$value\" source=\"$oFile:$lineNumber\" realpath=\"$realpath\"/>\n";
        }
#        print " <property name=\"$property\">\n";
#        print "  <typetag>$typetag</typetag>\n";
#        print "  <value>$value</value>\n";
#        print "  <source>$source</source>\n";
#        print " </property>\n";
    }


}
#########   Types  ##########
sub getTypeTags {
    my ($file) = @_;
    $count++;
    my $path = "$file";

    my @fileTypeTags = &getFileTypeTags($path);
    if (@fileTypeTags){ 
        foreach $typetag (@fileTypeTags){
            if ($typetag eq "") {next}
            if ($debug)
                {print("$typetag ($file)\n")}
            push @typeTags, $typetag;
        }
    }
    return;
}

sub getFileTypeTags {
    my ($file) = @_;
    open INPUT, "$file" or die "Unable to open $file";
    my $typeTag;
    my $found = 0;
    my $lineNumber=0;
    $comment = 0;
    my @fileTypeTags;
    $i=0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/NEW_TYPE_TAG/){
            my $line = &getFullLine;
            $typeTag = &getName($line);
#print "typetag=$typeTag\n";
            if ($typeTagFiles{$typeTag}) {
                print("<!-- *** Warning: TypeTag \"$typeTag\"redefined at file $file:$referenceLineCount --> \n");
#                exit(1);
            } else {
                $typeTagFiles{$typeTag} = $file; 
                $typeTagLineNumber{$typeTag} = $referenceLineCount; 
            }
#            $fileTypeTags[$i++] = $typeTag;
            push @fileTypeTags, $typeTag;
            $found = 1;
        }
    }
    close INPUT;
    if (not $found) {
        return undef
    }
    return @fileTypeTags;
}

sub getName {
    my $a, $b, $c;
    my($string) = @_;
    ($a, $b) = split /\(/,$string,2;
    if ($b =~ m/,/g){($c, $a) = split /,/,$b,2}
    else {($c, $a) = split /\)/, $b,2}
    $c =~ s/^\s+//;
    return $c;
}

#########   Inherits  ##########
sub getTypeInherits{
    my($file) = @_;
    my $path = "$file";


    open INPUT, "$path" or die "Unable to open $path";
    $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/NEW_TYPE_TAG/){
            my $line = &getFullLine;
            my $typeTag = &getName($line);
            if (/INHERITS_FROM/){
                my @d = &getInherits($typeTag, $line);
                foreach $a (@d){
                    if ($debug)
                      {print "get inherits $typeTag --> $a ($file:$referenceLineCount\n"}
                    push(@{ $inherits{"$typeTag"} }, "$a");
                }
           }
            next;
        }
    }
    close INPUT;
    return;
}

sub getInherits {
    my $typeTag;
    my $string;
    ($typeTag, $string) = @_;
    my $a, $b, $c, @d;
    undef @d;
    ($a, $b) = split /INHERITS_FROM/,$string,2;
    ($a, $c) = split /\(/, $b,2;
    ($a, $b) = split /\)/, $c,2;

    if ($a =~ m/,/){ @d = split /,/, $a} else {$d[0] = $a}
    my $i = 0;
#    print "string=$string";
    foreach $a (@d){ 
#        print "$typeTag --> $a\n"; 
        $d[$i] =~ s/^\s+//;
        $i++;
    }

   return @d;
}

sub printInherits{
    if ($debug){print("INHERITS:\n")}
    my $key;
    my @keys = keys(%inherits);
    foreach $key (@keys){
        if ($debug){print $key}
        my @array = @{ $inherits{$key} };
        &printFileArray(@array);
        if ($debug){print "\n"}
    }

}

# FIXME: not returning any inheritsString...
sub getInheritString{
    my ($typetag) = @_;
    my $key;
    my $inheritString = "";
    my @array = @{ $inherits{$typetag} };
    foreach $key (@array){
        $inheritString .= "$key, ";
    }
    return $inheritString;

}


#########   Properties  ##########
sub getProperties {
    my ($file) = @_;
    $count++;
    my $path = "$file";


    my @properties = &getFileProperties($path);
    foreach $property (@properties){
        push(@{ $properties{"$property"} }, "$file");
        $propertySource{$property}=$file;
        $propertyValues{$property}="undefined";
    }
    return;
}

sub getFileProperties{
    my ($path) = @_;

    open INPUT, "$path" or die "Unable to open $path";
    my @properties;
    my $i=0;
    $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/NEW_PROP_TAG/){
            my $line = &getFullLine;
            $prop = &getName($line);
            $properties[$i++] = $prop;
            $propertyLineNumber{$prop}=$referenceLineCount;
        }
    }
    close INPUT;
    if ($i == 0) {
       $properties[$i++] = "User Class Property";
    }
    return @properties;
}

sub printProperties {
    if ($debug){print("PROPERTIES:\n")}
    my $property;
    my @keys = keys(%properties);
    foreach $property (@keys){
        if ($debug){print $property}
        my @array = @{ $properties{"$property"} };
        &printFileArray(@array);
        if ($debug){print "\n"}
    }
}

sub printFileArray {
    my @a = @_;
    if ($debug){foreach $a (@a){ print " ($a)"}}
}

###############  Property values  ########
sub getPropertyValues {
    my ($file) = @_;
    my $path = "$file";

    open INPUT, "$path" or die "Unable to open $path";
    $comment = 0;
    $referenceLineCount=0;
    while (<INPUT>){
        $referenceLineCount++;
        s/^\s+//;
        if (/^#/ or /^\/\//){next}
        if (/\/\*/) {$comment = 1}
        if (/\*\//) {$comment = 0}
        if ($comment){next}
        if (/SET_TYPE_PROP/ or /SET_INT_PROP/ or /SET_BOOL_PROP/ or /SET_SCALAR_PROP/){
            my $line = &getFullLine;
            my $typetag = &getName($line);
            my $property = &getProperty($line);
            my $propValue = &getValue($line);

if ($debug){
            if (/SET_BOOL_PROP/ and /Gravity/ ) {
                print "SET_BOOL_PROP: $file --> \n$_";
                print "property=$property value=$propValue\n";
            }
            

            if ($propertyValues{$property}) {
                print("*** Warning: overwritting $property: \n");
                print("*** old value=$propertyValues{$property} new value=$propValue\n");
                print("*** old typetag=$propertyTypeTag{$property} new typetag=$typetag\n");
                print("*** old source=$propertySource{$property} new source=$file\n");
            }}
            $propertyValues{$property} = $propValue;
            $propertyTypeTag{$property} = $typetag;
            $propertySource{$property} = $file;
            $propertyLineNumber{$property} = $referenceLineCount;

            if ($verbose) {print("PROP ($propertySource{$property}) $propertyTypeTag{$property}:$property = $propertyValues{$property}\n")}
        }
    }
    close INPUT;
    return;
}

sub getProperty {
    my @a;
    my($string) = @_;
    @a = split /,/,$string,3;
    $a[1] =~ s/^\s+//;
    return $a[1];
}

sub getValue {
    my @a, $b, $c;
    my($string) = @_;
    @a = split /,/,$string, 3;
    $a[2] =~ s/^\s+//;
    chop $a[2]; # \n
    chop $a[2]; # ;
    $a[2] =~ s/^\s+//;
    return $a[2];
}

###############   General stuff ##########
sub getFullLine{
    my $b;
    my $line;
    if (/\/\//) {($line, $b) = split /\/\//, $_, 2}
    else {$line = $_}
    $line =~ s/\n//g;
    $line =~ s/^\s+//;
loop:
    if (not $line =~ m/;/g){
        my $nextLine = <INPUT>;
        $referenceLineCount++;
        $nextLine =~ s/\n//g;
        $nextLine =~ s/^\s+//;
        $line = $line . $nextLine;
        goto loop;
    }
    return $line;
}
