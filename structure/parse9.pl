#!/usr/bin/perl
use File::Basename;

if (not $ARGV[0]) {
    print "Please specify target file.\n";
    exit 1;
}


$installationPath = &getInstallationPath($ARGV[0]);
# Dumux installation directory may be specified directly:
#$installationPath= "/home/edscott/GIT/Beck2019a/beck2019a_decoupled";

#extra includes not in dumux installation path nor source directory:
#$includePath= "/home/edscott/tmp";
if (not $includePath){
    print "<!-- Additional template directory not specified -->\n";
}

@files;
%files;
$filecount=0;
%includes;
$startFile = $ARGV[0];
if ($ARGV[1]) {$problemTypeTag = $ARGV[1]}
else {
    &getProblemTypeTag($ARGV[0]);
}


$currentDir = `pwd`; chop $currentDir;

&openFileSub($startFile, dirname($startFile), "-");
@reversed = reverse @files;
@files = @reversed;
#foreach $f (@files){ print "$f\n"; }

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
}

# Find Dumux installation path from current file...
sub getInstallationPath {
    my ($infile) = @_;
    my $currentDir = dirname($infile);
    my $realpath = &realpath($currentDir);
    my $test = $realpath . "/dumux";
    if (-d $test) {
        print "<!-- Assuming Dumux templates at \"$test\" -->\n";
        return $realpath
    }
    if ($realpath eq "/") {
        print "<!-- *** Warning: Dumux template source directory not found-->\n";
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
    $oFile =~ s/$installationPath\///;
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
    my ($path, $dirname) = @_;
#    print "parsing $path (at $dirname) ...\n";
    
    if ($includes{$path}) {return}
    $includes{$path} = 1;
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
    if (/Calcite/){print}
# allow incorrect include path with "" instead of <>
        if  (/"dumux\//)   {
            ($a, $b, $c) = split /"/, $_, 3;
            $b =~ s/^\s+//;
            $d = $installationPath . "/" . $b;
#                print "d = $d\n";
            $dirname = dirname($d);
            &openFileSub($d, $dirname, $path);          
            next;
        }
        if (/dumux/ and /</ and />/)  {
            ($a, $b) = split /</, $_, 2;
#                print "a b = $a $b\n";
            ($a, $c) = split />/, $b, 2;
#                print "a c = $a $c\n";
            $a =~ s/^\s+//;
            $d = $installationPath . "/" . $a;
#                print "d = $d\n";
            $dirname = dirname($d);
            &openFileSub($d, $dirname, $path);          
            next;
        }
        if (/"/) {

#           relative includes...
            ($a, $b, $c) = split /"/, $_, 3;
#                print "a b c = $a $b $c\n";
            $b=~ s/^\s+//;
            $dirname = dirname($path);
            my $r = $b;
            $d = $dirname . "/" . $b;
#           if it exists, use it, if not, test include path
#
#            print "relative d = $d (called from $path)\n";
            if (not -e $d){
                if ($includePath) {$d = $includePath . "/" . $d;}
                else {$d = $installationPath . "/" . $d;}
                if (-e $d) {
                    &openFileSub($d, dirname($d), $path);          
                    next;
                }
                else {
                    if (-e "/usr/include/$r"){
# skip incorrect "" includes.
                        print "<!-- skipping header /usr/include/$r -->\n";
                        next;
                    }
#                    print $_;
#                    print "r d = $r (called from $path)\n";
                    print "<!-- *** Warning could not find $r for inclusion -->\n";
                    next;
#                    print "Abort...\n"; exit(1)
                }
            }
            &openFileSub($d, dirname($d), $path);          
            next;
        }

    }
    close $stream;
}

sub openFileSub {
    my ($d, $dirname, $path) = @_;
    if ($fileList{$d}){
#                print "*** $d already included...\n";
    } else {
#               print "3. $path --> $d\n";
        $fileList{$d} = 1;
        push(@files, $d);
        push(@{ $files{$path} }, $d);
        &openFile($d, $dirname);    
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
<structure source=\"$ARGV[0]\" templates=\"$installationPath/dumux\" $extraIncludes/>
EOF
&printFilesXML($startFile, 0);
&printPropertiesXML;
&printTypeTagsXML;
print OUTPUT "</dumux-info>\n";
}

sub realpath{
    my ($inPath) = @_;
    if (not -e $inPath){
        if (-e $installationPath."/$inPath"){ $inPath = $installationPath."/$inPath"}
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
        $oFile =~ s/$installationPath\///;
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
                    $oFile =~ s/$installationPath\///;
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
        $oFile =~ s/$installationPath\///;
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
