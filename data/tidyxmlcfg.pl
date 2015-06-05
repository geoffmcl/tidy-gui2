#!/usr/bin/perl -w
# NAME: tidyxmlcfg.pl
# AIM: Examine the xml config file
use strict;
use warnings;
use File::Basename;  # split path ($name,$dir,$ext) = fileparse($file [, qr/\.[^.]*/] )
use Cwd;
my $os = $^O;
my $perl_dir = '/home/geoff/bin';
my $PATH_SEP = '/';
my $temp_dir = '/tmp';
if ($os =~ /win/i) {
    $perl_dir = 'C:\GTools\perl';
    $temp_dir = $perl_dir;
    $PATH_SEP = "\\";
}
unshift(@INC, $perl_dir);
require 'lib_utils.pl' or die "Unable to load 'lib_utils.pl' Check paths in \@INC...\n";
# log file stuff
our ($LF);
my $pgmname = $0;
if ($pgmname =~ /(\\|\/)/) {
    my @tmpsp = split(/(\\|\/)/,$pgmname);
    $pgmname = $tmpsp[-1];
}
my $outfile = $temp_dir.$PATH_SEP."temp.$pgmname.txt";
open_log($outfile);

# user variables
my $VERS = "0.0.5 2015-01-09";
my $load_log = 0;
my $in_file = '';
my $in_file2 = '';
my $in_file3 = '';
my $verbosity = 0;
my $out_file = '';

my $write_tmp_file = 0;
my $tmp_out2 = $temp_dir.$PATH_SEP."temptg.h";
my $tmp_out3 = $temp_dir.$PATH_SEP."temptg.cpp";


# ### DEBUG ###
my $debug_on = 1;
my $def_file = 'C:\Projects\qt\qt-gui\src\tidy-gui\data\tidycfg.xml';
my $def_file2 = 'C:\Projects\qt\qt-gui\src\tidy-gui\tg-dialog.h';
my $def_file3 = 'C:\Projects\qt\qt-gui\src\tidy-gui\tg-dialog.cpp';


### program variables
my @warnings = ();
my $cwd = cwd();

sub VERB1() { return $verbosity >= 1; }
sub VERB2() { return $verbosity >= 2; }
sub VERB5() { return $verbosity >= 5; }
sub VERB9() { return $verbosity >= 9; }

sub show_warnings($) {
    my ($val) = @_;
    if (@warnings) {
        prt( "\nGot ".scalar @warnings." WARNINGS...\n" );
        foreach my $itm (@warnings) {
           prt("$itm\n");
        }
        prt("\n");
    } else {
        prt( "\nNo warnings issued.\n\n" ) if (VERB9());
    }
}

sub pgm_exit($$) {
    my ($val,$msg) = @_;
    if (length($msg)) {
        $msg .= "\n" if (!($msg =~ /\n$/));
        prt($msg);
    }
    show_warnings($val);
    close_log($outfile,$load_log);
    exit($val);
}


sub prtw($) {
   my ($tx) = shift;
   $tx =~ s/\n$//;
   prt("$tx\n");
   push(@warnings,$tx);
}

my $cppstatic = "// static objects\n";
my @chkboxes = ();
my @labels = ();
my @editboxes = ();
my @combos = ();
my $addcode = "// code to be added\n";
my $addhead = "// code to be added to header\n";
my $lasthead = '';
my %slothash = ();

# 1. create tab class
# class GeneralTab : public QWidget
# {
#    Q_OBJECT
# public:
#    GeneralTab(const QFileInfo &fileInfo, QWidget *parent = 0);
#};
sub get_tab_class($) {
    my $name = shift;
    my $txt = <<EOF;

class $name : public QWidget
{
    Q_OBJECT
public:
    $name( PINFOSTR pinf, QWidget *parent = 0);
};
EOF
    return $txt;
}

sub get_imp_head($) {
    my $name = shift;
    my $txt = <<EOF;

$name\:\:$name( PINFOSTR pinf, QWidget *parent)
    : QWidget(parent)
{
    int i;
    QString s;
EOF
    return $txt;
}

sub set_header_slots($$) {
    my ($token,$name) = @_;
    my $txt = '';
    if ($lasthead ne $name) {
        $lasthead = $name;
        $txt .= "public slots: // ADD to $name\n";
    }
    $txt .= "    void on_$token();\n";
    $addhead .= $txt;
    $slothash{$name} = [] if (! defined $slothash{$name});
    my $ra = $slothash{$name};
    push(@{$ra},$txt);
}


sub get_bool_change($$$$) {
    my ($label,$token,$class,$name) = @_;
    set_header_slots($token,$name);
    my $txt = <<EOF;

void $name\:\:on_$token()
{
    const char *label = "$label";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b = (Bool)w->isChecked();
        setConfigBool( label, b );
    }
}

EOF
    return $txt;
}

#    QCheckBox *$token = new QCheckBox(\"$label\"));
#    if (fileInfo.isReadable())
#        readable->setChecked(true);
sub get_bool_cb($$$$) {
    my ($label,$token,$class,$name) = @_;
    my $text = "$label (Boolean)";
    $cppstatic .= "static QCheckBox *$token;\n";
    push(@chkboxes, [$token,$label,'Boolean',$class]);
    $addcode .= get_bool_change($label,$token,$class,$name);
    my $txt = <<EOF;

    $token = new QCheckBox(\"$text\");
    if (getConfigBool(\"$label\")) {
        $token->setChecked(true);
    }
    connect($token,SIGNAL(clicked()),this,SLOT(on_$token()));

EOF
    return $txt;
}

sub get_autobool_change($$$$) {
    my ($label,$token,$class,$name) = @_;
    set_header_slots($token,$name);
    my $txt = <<EOF;

void $name\:\:on_$token()
{
    const char *label = "$label";
    QCheckBox *w = qobject_cast<QCheckBox *>(sender());
    if (w) {
        Bool b;
        Qt::CheckState cs = w->checkState();
        if (cs ==  Qt::Checked)
            b = yes;
        else if (cs == Qt::Unchecked)
            b = no;
        else
            b = (Bool)2;
        setConfigABool( label, b );
    }
}

EOF
    return $txt;
}


sub get_bool_cb3($$$$$) {
    my ($label,$token,$type,$class,$name) = @_;
    my $text = "$label (AutoBool)";
    $cppstatic .= "static QCheckBox *$token;\n";
    push(@chkboxes, [$token,$label,$type,$class]);
    $addcode .= get_autobool_change($label,$token,$class,$name);
    my $txt = <<EOF;

    $token = new QCheckBox(\"$text\");
    $token->setTristate(true);
    i = getConfigABool(\"$label\");
    if (i == 0) {
        $token->setChecked(false);
        // $token->setCheckState(Qt::Unchecked); // 0
    } else if (i == 1) {
        $token->setChecked(true);
        // $token->setCheckState(Qt::Checked); // 2
    } else {
        $token->setCheckState(Qt::PartiallyChecked); // 1
    }
    connect($token,SIGNAL(clicked()),this,SLOT(on_$token()));

EOF
    return $txt;
}

# $addcode .= get_editint_change($label,$token,$type,$class,$name,$te);
sub get_editint_change($$$$$$) {
    my ($label,$token,$type,$class,$name,$te) = @_;
    set_header_slots($te,$name);
    my $txt = <<EOF;

void $name\:\:on_$te()
{
    const char *label = "$label";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        int i = s.toInt();
        setConfigInt( label, i );
    }
}

EOF
    return $txt;
}

# $cpp .= get_edit_int($label,$token,$type,$class,$name);
sub get_edit_int($$$$$) {
    my ($label,$token,$type,$class,$name) = @_;
    my $te = $token."Ed";
    my $text = "$label (Integer)";
    $cppstatic .= "static QLabel *$token;\n";
    $cppstatic .= "static QLineEdit *$te;\n";
    push(@labels, [$token,$label,$type,$class]);
    push(@editboxes, [$te,$label,$type,$class]);
    $addcode .= get_editint_change($label,$token,$type,$class,$name,$te);
    my $txt = "\n";
    $txt .= "    $token = new QLabel(\"$text\");\n";
    $txt .= "    i = getConfigInt(\"$label\");\n";
    $txt .= "    s = QString::number(i);\n";
    $txt .= "    $te = new QLineEdit(s);\n";
    $txt .= "    $te->setMaximumWidth(50);\n";
    $txt .= "    connect($te,SIGNAL(editingFinished()),this,SLOT(on_$te()));\n";
    return $txt;
}

sub get_editstg_change($$$$$$) {
    my ($label,$token,$type,$class,$name,$te) = @_;
    set_header_slots($te,$name);
    my $txt = <<EOF;

void $name\:\:on_$te()
{
    const char *label = "$label";
    QLineEdit *w = qobject_cast<QLineEdit *>(sender());
    if (w) {
        QString s = w->text();
        s = s.trimmed();
        setConfigStg( label, s.toStdString().c_str() );
    }
}

EOF
    return $txt;
}

sub get_edit_stg($$$$$) {
    my ($label,$token,$type,$class,$name) = @_;
    my $te = $token."Ed";
    my $text = "$label (String)";
    $cppstatic .= "static QLabel *$token;\n";
    $cppstatic .= "static QLineEdit *$te;\n";
    push(@labels, [$token,$label,$type,$class]);
    push(@editboxes, [$te,$label,$type,$class]);
    $addcode .= get_editstg_change($label,$token,$type,$class,$name,$te);
    my $txt = "\n";
    $txt .= "    $token = new QLabel(\"$text\");\n";
    $txt .= "    s = getConfigStg(\"$label\");\n";
    $txt .= "    $te = new QLineEdit(s);\n";
    $txt .= "    connect($te,SIGNAL(editingFinished()),this,SLOT(on_$te()));\n";
    return $txt;
}

sub get_combo_change1($$$) {
    my ($name,$comb,$label) = @_;
    set_header_slots($comb,$name);
    my $txt = <<EOF;

// combobox slot change handler
void $name\:\:on_$comb()
{
    const char *label = "$label";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnc( label, qs.toStdString().c_str() );
    }
}

EOF
    return $txt;
}

sub get_combo_change2($$$) {
    my ($name,$comb,$label) = @_;
    set_header_slots($comb,$name);
    my $txt = <<EOF;

// combobox slot change handler
void $name\:\:on_$comb()
{
    const char *label = "$label";
    QComboBox *w = qobject_cast<QComboBox *>(sender());
    if (w) {
        int i = w->currentIndex();
        QVariant qv = w->itemData(i);
        QString qs = qv.toString();
        setConfigEnum( label, qs.toStdString().c_str() );
    }
}

EOF
    return $txt;
}


sub get_combo($$$$$$) {
    my ($label,$token,$exam,$typ,$class,$name) = @_;
    my @arr = split(",",$exam);
    my $cnt = scalar @arr;
    my $cb = $token."Comb";
    my $lo = $token."Lay";
    my $comb = "// TODO: handle $typ change\n";
    my $conn = "// TODO: handle $typ connect\n";
    if ($cnt == 0) {
        pgm_exit(1,"ERROR: combo failed $label, $token, $exam! *** FIX ME ***\n");
    }
    my ($itm);
    $cppstatic .= "static QComboBox *$cb;\n";
    push(@combos, [$cb,$label,$typ,$class]);
    my $txt = "\n";
    $txt .= "    QGroupBox *$token = new QGroupBox(\"$label\");\n";
    $txt .= "    $cb = new QComboBox();\n";
    foreach $itm (@arr) {
        $itm = trim_all($itm);
        $txt .= "    $cb->addItem(\"$itm\",\"$itm\");\n";
    }
    if ($typ eq 'Encoding') {
        $txt .= "    s = getConfigEnc(\"$label\");\n";
        $comb = get_combo_change1($name,$cb,$label);
        $conn = "    connect($cb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_$cb()));\n";
    } elsif ($typ eq 'enum') {
        $txt .= "    s = getConfigEnum(\"$label\");\n";
        $comb = get_combo_change2($name,$cb,$label);
        $conn = "    connect($cb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_$cb()));\n";
    } elsif ($typ eq 'DocType') {
        $txt .= "    s = getConfigPick(\"$label\");\n";
        # may be the same???
        $comb = get_combo_change2($name,$cb,$label);
        $conn = "    connect($cb, SIGNAL(currentIndexChanged(int)), this, SLOT(on_$cb()));\n";
    } else {
        pgm_exit(1,"ERROR: get_combo() called with unknonw type '$typ'! *** FIX ME ***\n");
    }
    $txt .= "    i = $cb->findData(s);\n";
    $txt .= "    if (i != -1) {\n";
    $txt .= "        $cb->setCurrentIndex(i);\n";
    $txt .= "    }\n";
    $txt .= "    QVBoxLayout *$lo = new QVBoxLayout;\n";
    $txt .= "    $lo->addWidget($cb);\n";
    $txt .= "    $lo->addStretch(1);\n";
    $txt .= "    $token->setLayout($lo);\n";
    ###$txt .= "    // this FAILED ???\n";
    ###$txt .= "    // connect($cb, SIGNAL(currentIndexChanged(int)), this, SLOT($comb(\"$label\")));\n";
    $txt .= $conn;
    $addcode .= $comb;
    return $txt;
}

sub get_tab_name($) {
    my $name = shift;
    $name = uc(substr($name,0,1)).substr($name,1)."Tab";
    return $name;
}

sub get_lo_name($) {
    my $name = shift;
    $name = uc(substr($name,0,1)).substr($name,1)."Layout";
    return $name;
}

sub get_strucs() {
    my $txt = <<EOF;

// structures for objects
typedef struct tagCHKBXS {
    QCheckBox **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}CHKBXS, *PCHKBXS;

typedef struct tagLABEL {
    QLabel **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}LABEL, *PLABEL;

typedef struct tagLINED {
    QLineEdit **p;   // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}LINED, *PLINED;

typedef struct tagCOMBOS {
    QComboBox **p;  // pointer to pointer
    const char *lab;
    const char *typ;
    const char *tab;
}COMBOS, *PCOMBOS;

EOF
    return $txt;
}

sub reset_in_file3($) {
    my $newcpp = shift;
    my $inf = $in_file3;
    if (! open INF, "<$inf") {
        prtw("WARNING: Failed to open '$inf'\n"); 
        return;
    }
    my @lines = <INF>;
    close INF;
    my $lncnt = scalar @lines;
    prt("Processing $lncnt lines, from [$inf]...\n");
    my ($i,$line,$nline,$ra);
    my @nlines = ();
    for ($i = 0; $i < $lncnt; $i++) {
        $line = $lines[$i];
        if ($line =~ /^\/\/\s+INSERT\s+HERE\s+/) {
            push(@nlines,$line);
            $i++;
            for (; $i < $lncnt; $i++) {
                $line = $lines[$i];
                if ($line =~ /^\/\/\s+END\s+INSERT\s+/) {
                    last;
                }
            }
            last;
        }
        push(@nlines,$line);
    }
    $nline = join("",@nlines);
    $nline .= $newcpp;
    ### $nline .= $line;
    for (; $i < $lncnt; $i++) {
        $line = $lines[$i];
        $nline .= $line;
    }
    if ($write_tmp_file) {
        write2file($nline,$tmp_out3);
        prt("New contents of $inf\nwritten to $tmp_out3\n");
    } else {
        rename_2_old_bak($inf);
        write2file($nline,$inf);
        prt("New contents of $inf written\n");
    }
}


sub reset_in_file2() {
    my $inf = $in_file2;
    if (! open INF, "<$inf") {
        prtw("WARNING: Failed to open '$inf'\n"); 
        return;
    }
    my @lines = <INF>;
    close INF;
    my $lncnt = scalar @lines;
    prt("Processing $lncnt lines, from [$inf]...\n");
    my ($i,$line,$class,$ra);
    my @nlines = ();
    $class = 'unknonw';
    for ($i = 0; $i < $lncnt; $i++) {
        $line = $lines[$i];
        chomp $line;
        if ($line =~ /^class\s+(\w+)\s+/) {
            $class = $1;
        } elsif ($line =~ /^public\s+slots:/) {
            if (defined $slothash{$class}) {
                $i++;
                for (; $i < $lncnt; $i++) {
                    $line = $lines[$i];
                    chomp $line;
                    last if ($line =~ /^\};/);
                }
                $ra = $slothash{$class};
                foreach $line (@{$ra}) {
                    $line = trim_tailing($line);
                    push(@nlines,$line);
                }
                push(@nlines,"");
                push(@nlines,"};");
                next;
            }
        }
        push(@nlines,$line);
    }
    $line = join("\n",@nlines);
    $line .= "\n";
    if ($write_tmp_file) {
        write2file($line,$tmp_out2);
        prt("New contents of $inf\nwritten to $tmp_out2\n");
    } else {
        rename_2_old_bak($inf);
        write2file($line,$inf);
        prt("New contents of $inf written\n");
    }

}


#     QLabel *fileNameLabel = new QLabel(tr("File Name:"));
#     QLineEdit *fileNameEdit = new QLineEdit(fileInfo.fileName());

sub show_options($) {
    my ($ra) = @_;  # \@options);
    my $len = scalar @{$ra};
    my ($rh,$op,$val,$class,$type,$def,$name,$tt,$label,$token,$lon,$hcnt,$roa,$i,$exam,$curr);
    my @opts = qw( name type default example seealso description );
    my %classes = ();
    $op = 'class';
    foreach $rh (@{$ra}) {
        if (defined ${$rh}{$op}) {
            $val = ${$rh}{$op};
            $classes{$val} = 1;
        }
    }
    my @arr = sort keys %classes;
    my $cnt = scalar @arr;
    prt("Have $len options, $cnt classes - ".join(" ",@arr)."\n");
    my $cpp = '';
    my $chh = '';
    my @checkboxes = ();
    foreach $class (@arr) {
        prt("\nOption class '$class'\n");
        $name = get_tab_name($class);
        $lon  = get_lo_name($class);
        $chh .= get_tab_class($name);
        $cpp .= get_imp_head($name);
        @checkboxes = ();   # clear widget to add
        foreach $rh (@{$ra}) {
            $curr  = ${$rh}{class};
            $type  = ${$rh}{type};
            $def   = ${$rh}{default};
            $label = ${$rh}{name};
            $tt    = ${$rh}{description};
            $exam  = ${$rh}{example};
            $token = $label;
            if ($curr ne $class) {
                # UGH - Want to switch some page classes
                if (($curr eq 'markup') && ($class eq 'encoding') &&
                    (($label eq 'doctype')||($label eq 'repeated-attributes')) ) {
                    # add this markup to encoding page
                } elsif (($curr eq 'markup') && ($class eq 'misc') &&
                    (($label eq 'alt-text') ||
                     ($label eq 'css-prefix') ||
                     ($label =~ /^new-(.+)-tags$/)) ) {
                    # add this markup to misc page
                } else {
                    next;
                }
            } else {
                # $class and $curr are the SAME
                # have moved two to another page
                if (($curr eq 'markup') &&
                    (($label eq 'doctype')||($label eq 'repeated-attributes')||
                        ($label eq 'alt-text') ||
                        ($label eq 'css-prefix') ||
                        ($label =~ /^new-(.+)-tags$/) ) ) {
                    next;   # these have been MOVED to 'encoding' or 'misc'
                    # mainly due to 'markup' has too many options
                } elsif (($curr eq "misc")&&($label eq "output-file")) {
                    next; # output file moved to General (Main) Tab
                }

            }
            $token =~ s/\-/_/g;
            if ($type eq 'Boolean') {
                $cpp .= get_bool_cb($label,$token,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
            } elsif ($type eq 'Integer') {
                $cpp .= get_edit_int($label,$token,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
                push(@checkboxes,[$token.'Ed',$type,$label]);   # save to add to layout
            } elsif ($type eq 'AutoBool') {
                $cpp .= get_bool_cb3($label,$token,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
            } elsif ($type eq 'String') {
                $cpp .= get_edit_stg($label,$token,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
                push(@checkboxes,[$token.'Ed',$type,$label]);   # save to add to layout
            } elsif ($type eq 'Tag names') {
                $cpp .= get_edit_stg($label,$token,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
                push(@checkboxes,[$token.'Ed',$type,$label]);   # save to add to layout
            } elsif ($type eq 'Encoding') {
                $cpp .= get_combo($label,$token,$exam,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
            } elsif ($type eq 'enum') {
                $cpp .= get_combo($label,$token,$exam,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
            } elsif ($type eq 'DocType') {
                $cpp .= get_combo($label,$token,$exam,$type,$class,$name);
                push(@checkboxes,[$token,$type,$label]);   # save to add to layout
            } else {
                prt("\n");
                foreach $op (@opts) {
                    if (defined ${$rh}{$op}) {
                        $val = ${$rh}{$op};
                        prt("$op: $val\n");
                    }
                }
            }
        }
        $cnt = scalar @checkboxes;
        if ($class eq 'markup') {
            $hcnt = int($cnt / 2);
            my $n1 = $lon."1";
            my $n2 = $lon."2";
            $cpp .= "\n";
            $cpp .= "    QVBoxLayout *$n1 = new QVBoxLayout;\n";
            for ($i = 0; $i < $hcnt; $i++) {
                $roa = $checkboxes[$i];
                $token = ${$roa}[0];
                $type  = ${$roa}[1];
                $label = ${$roa}[2];
                $cpp .= "    $n1->addWidget($token);\n";
            }
            $cpp .= "    $n1->addStretch(1);\n";
            $cpp .= "\n";
            $cpp .= "    QVBoxLayout *$n2 = new QVBoxLayout;\n";
            for (; $i < $cnt; $i++) {
                $roa = $checkboxes[$i];
                $token = ${$roa}[0];
                $type  = ${$roa}[1];
                $label = ${$roa}[2];
                $cpp .= "    $n2->addWidget($token);\n";
            }
            $cpp .= "    $n2->addStretch(1);\n";
            $cpp .= "\n";
            $cpp .= "    QHBoxLayout *$lon = new QHBoxLayout;\n";
            $cpp .= "    $lon->addLayout($n1);\n";
            $cpp .= "    $lon->addLayout($n2);\n";
            $cpp .= "    setLayout($lon);\n";
        } else {
            $cpp .= "\n";
            $cpp .= "    QVBoxLayout *$lon = new QVBoxLayout;\n";
            for ($i = 0; $i < $cnt; $i++) {
                $roa = $checkboxes[$i];
                $token = ${$roa}[0];
                $type  = ${$roa}[1];
                $label = ${$roa}[2];
                $cpp .= "    $lon->addWidget($token);\n";
            }
            $cpp .= "    $lon->addStretch(1);\n";
            $cpp .= "    setLayout($lon);\n";
        }
        $cpp .= "}\n\n";
    }

    my $tm = time();
    my $newcpp = "\n// Generated by $pgmname ".lu_get_YYYYMMDD_hhmmss_UTC($tm)." UTC, ".lu_get_hhmmss($tm)." local\n\n";
    #prt($chh);
    prt($newcpp);
    prt($addhead);
    prt($addcode);
    $newcpp .= $addcode;
    prt(get_strucs());
    $newcpp .= get_strucs();
    prt($cppstatic);
    $newcpp .= $cppstatic;
    # ==================
    # CHKBXS
    my %h = ();
    foreach $roa (@chkboxes) {
        $type  = ${$roa}[2];
        $h{$type} = 1;
    }
    @arr = sort keys %h;
    $name = "\n";
    $name .= "// CheckBox types: ".join(" ",@arr)."\n";
    $name .= "static CHKBXS chkbxs[] = {\n";
    foreach $roa (@chkboxes) {
        $token = ${$roa}[0];
        $label = ${$roa}[1];
        $type  = ${$roa}[2];
        $class = ${$roa}[3];
        $name .= "    { &$token, \"$label\", \"$type\", \"$class\" },\n";
    }
    $name .= "    // LAST ENTRY\n";
    $name .= "    { 0, 0, 0, 0 }\n";
    $name .= "};\n";
    # LABEL
    %h  = ();
    foreach $roa (@labels) {
        $type  = ${$roa}[2];
        $h{$type} = 1;
    }
    @arr = sort keys %h;
    $name .= "\n";
    $name .= "// Label types: ";
    foreach $type (@arr) {
        $name .= "'$type' ";
    }
    $name .= "\n";
    $name .= "static LABEL label[] = {\n";
    foreach $roa (@labels) {
        $token = ${$roa}[0];
        $label = ${$roa}[1];
        $type  = ${$roa}[2];
        $class = ${$roa}[3];
        $name .= "    { &$token, \"$label\", \"$type\", \"$class\" },\n";
    }
    $name .= "    // LAST ENTRY\n";
    $name .= "    { 0, 0, 0, 0 }\n";
    $name .= "};\n";
    # LINED 
    %h = ();
    foreach $roa (@editboxes) {
        $type  = ${$roa}[2];
        $h{$type} = 1;
    }
    @arr = sort keys %h;
    $name .= "\n";
    $name .= "// LineEdit types: ";
    foreach $type (@arr) {
        $name .= "'$type' ";
    }
    $name .= "\n";
    $name .= "static LINED lined[] = {\n";
    foreach $roa (@editboxes) {
        $token = ${$roa}[0];
        $label = ${$roa}[1];
        $type  = ${$roa}[2];
        $class = ${$roa}[3];
        $name .= "    { &$token, \"$label\", \"$type\", \"$class\" },\n";
    }
    $name .= "    // LAST ENTRY\n";
    $name .= "    { 0, 0, 0, 0 }\n";
    $name .= "};\n";
    # COMBOS
    %h = ();
    foreach $roa (@combos) {
        $type  = ${$roa}[2];
        $h{$type} = 1;
    }
    @arr = sort keys %h;
    $name .= "\n";
    $name .= "// ComboBox types: ";
    foreach $type (@arr) {
        $name .= "'$type' ";
    }
    $name .= "\n";
    $name .= "static COMBOS combos[] = {\n";
    foreach $roa (@combos) {
        $token = ${$roa}[0];
        $label = ${$roa}[1];
        $type  = ${$roa}[2];
        $class = ${$roa}[3];
        $name .= "    { &$token, \"$label\", \"$type\", \"$class\" },\n";
    }
    $name .= "    // LAST ENTRY\n";
    $name .= "    { 0, 0, 0, 0 }\n";
    $name .= "};\n";
    prt($name);
    $newcpp .= $name;
    prt($cpp);
    $newcpp .= $cpp;
    if (-f $in_file2) {
        reset_in_file2();
    }
    if (-f $in_file3) {
        reset_in_file3($newcpp);
    }
    ### $load_log = 1;
}

sub process_in_file($) {
    my ($inf) = @_;
    if (! open INF, "<$inf") {
        pgm_exit(1,"ERROR: Unable to open file [$inf]\n"); 
    }
    my @lines = <INF>;
    close INF;
    my $lncnt = scalar @lines;
    prt("Processing $lncnt lines, from [$inf]...\n");
    my ($line,$inc,$lnn,$len,$tag,$txt,$intag,$i,$ch,$cn,$i2,$inquot,$qc,$last);
    $lnn = 0;
    $tag = '';
    $txt = '';
    $intag = 0;
    $inquot = 0;
    my @tagstack = ();
    my $inconfig = 0;
    my $inopt = 0;
    my $inname = 0;
    my $intype = 0;
    my $indef = 0;
    my $inex = 0;
    my $indesc = 0;
    my $inseealso = 0;
    my $inem = 0;
    my $incode = 0;
    my $inanch = 0;
    my $href = '';
    my %hash = ();  # for each option
    my @options = ();   # stack option hashes
    foreach $line (@lines) {
        chomp $line;
        $lnn++;
        $line = trim_all($line);
        $len = length($line);
        next if ($len == 0);
        for ($i = 0; $i < $len; $i++) {
            $i2 = $i + 1;
            $ch = substr($line,$i,1);
            if ($intag) {
                $tag .= $ch;
                if ($ch eq '>') {
                    if (VERB5()) {
                        prt($txt) if (length($txt));
                        prt($tag);
                    }
                    if ($tag =~ /^<\/(\w+)>/) {
                        $inc = $1;  # closing tag
                        prt("\n") if (VERB5());
                        if (@tagstack) {
                            $last = pop @tagstack;
                            if ($last ne $inc) {
                                prtw("WARNING: $lnn: Closing tag '$tag' NOT not last '$last' on stack\n");
                            } else {
                                if ($last eq 'config') {
                                    # <config version="4.9.30">
                                    $inconfig = 0;
                                } elsif ($last eq 'option') {
                                    # <option class="print">
                                    if ($inopt) {
                                        my %h = %hash;
                                        push(@options, \%h);
                                    }
                                    $inopt = 0;

                                } elsif ($last eq 'name') {
                                    # <name>indent-spaces</name>
                                    if ($inname && length($txt)) {
                                        $hash{name} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $inname = 0;
                                } elsif ($last eq 'type') {
                                    # <type>Integer</type>
                                    if ($intype && length($txt)) {
                                        $hash{type} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $intype = 0;
                                } elsif ($last eq 'default') {
                                    # <default>2</default>
                                    if ($indef && length($txt)) {
                                        $hash{default} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $indef = 0;
                                } elsif ($last eq 'example') {
                                    # <example>0, 1, 2, ...</example>
                                    if ($inex && length($txt)) {
                                        $hash{example} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $inex = 0;
                                } elsif ($last eq 'description') {
                                    # <description>This option ...</description>
                                    if ($indesc && length($txt)) {
                                        $hash{description} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $indesc = 0;
                                } elsif ($last eq 'seealso') {
                                    # <seealso>indent</seealso>
                                    if ($inseealso && length($txt)) {
                                        $hash{seealso} = $txt;
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $inseealso = 0;
                                } elsif ($last eq 'em') {
                                    # within description
                                    # AND within default
                                    if ($indef) {
                                        $txt = '<em>'.$txt.'</em>';
                                    } elsif ($indesc) {
                                        if ($inem && length($txt)) {
                                            $hash{descripion} .= ' <em>'.$txt.'</em>';
                                        } else {
                                            prtw("WARNING: $lnn: Close $last FAILED\n");
                                        }
                                    } else {
                                        prtw("WARNING: $lnn: Close $last NOT in default or description!\n");
                                    }
                                    $inem = 0;
                                } elsif ($last eq 'code') {
                                    # within description
                                    if ($incode && length($txt)) {
                                        $hash{descripion} .= ' <code>'.$txt.'</code>';
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $incode = 0;
                                } elsif ($last eq 'a') {
                                    # within description
                                    if ($inanch && length($txt)) {
                                        $hash{descripion} .= " $href$txt</a>";
                                    } else {
                                        prtw("WARNING: $lnn: Close $last FAILED\n");
                                    }
                                    $inanch = 0;
                                } else {
                                    pgm_exit(1,"$lnn: UNCODED close tag '$last'! *** FIX ME ***\n");
                                }
                            }
                        } else {
                            prtw("WARNING: $lnn: Closing tag '$tag' NOT on stack\n");
                        }
                    } elsif ($tag =~ /\/>$/) {
                        prt("\n") if (VERB5());
                    } else {
                        if ($tag =~ /^<\?/) {
                            # declaration
                            $txt = '';
                        } elsif ($tag =~ /^<config\s+/) {
                            push(@tagstack,'config');
                            $inconfig = 1;
                            # <config version="4.9.30">
                        } elsif ($tag =~ /^<option\s+/) {
                            push(@tagstack,'option');
                            %hash = (); # restart the hash
                            if ($tag =~ /\s+class=\"(\w+)\"/) {
                                $inc = $1;
                                $hash{class} = $inc;
                            } else {
                                pgm_exit(1,"$lnn: Failed option class '$tag'! *** FIX ME ***\n");
                            }
                            # <option class="print">
                            $inopt = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<name>$/) {
                            push(@tagstack,'name');
                            # <name>indent-spaces</name>
                            $inname = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<type>$/) {
                            push(@tagstack,'type');
                            # <type>Integer</type>
                            $intype = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<default>$/) {
                            push(@tagstack,'default');
                            # <default>2</default>
                            $indef = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<example>$/) {
                            push(@tagstack,'example');
                            # <example>0, 1, 2, ...</example>
                            $inex = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<description>$/) {
                            push(@tagstack,'description');
                            # <description>This option ...</description>
                            $indesc = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<seealso>$/) {
                            push(@tagstack,'seealso');
                            # <seealso>indent</seealso>
                            $inseealso = 1;
                            $txt = '';
                        } elsif ($tag =~ /^<em>$/) {
                            push(@tagstack,'em');
                            # within description
                            $inem = 1;
                        } elsif ($tag =~ /^<code>$/) {
                            push(@tagstack,'code');
                            # within description
                            $incode = 1;
                        } elsif ($tag =~ /^<a\s+/) {
                            push(@tagstack,'a');
                            # within description
                            $href = $tag;
                            $inanch = 1;
                        } else {
                            pgm_exit(1,"$lnn: Uncoded tag '$tag'! *** FIX ME ***\n");
                        }
                    }
                    ### $txt = '';
                    $intag = 0;
                }
            } else {
                if ($ch eq '<') {
                    $intag = 1;
                    $tag = $ch;
                } else {
                    $txt .= $ch;
                }
            }
        }
    }
    $len = scalar @options;
    show_options(\@options);
    $len = scalar @tagstack;
    if ($len) {
        prtw("WARNING: $len on stack! ".join(" ",@tagstack)."\n");
    } else {
        prt("Apppears a clean parse of $inf\n");
    }
    #$load_log = 1;
}

#########################################
### MAIN ###
parse_args(@ARGV);
process_in_file($in_file);
pgm_exit(0,"");
########################################

sub need_arg {
    my ($arg,@av) = @_;
    pgm_exit(1,"ERROR: [$arg] must have a following argument!\n") if (!@av);
}

sub parse_args {
    my (@av) = @_;
    my ($arg,$sarg);
    my $verb = VERB2();
    while (@av) {
        $arg = $av[0];
        if ($arg =~ /^-/) {
            $sarg = substr($arg,1);
            $sarg = substr($sarg,1) while ($sarg =~ /^-/);
            if (($sarg =~ /^h/i)||($sarg eq '?')) {
                give_help();
                pgm_exit(0,"Help exit(0)");
            } elsif ($sarg =~ /^v/) {
                if ($sarg =~ /^v.*(\d+)$/) {
                    $verbosity = $1;
                } else {
                    while ($sarg =~ /^v/) {
                        $verbosity++;
                        $sarg = substr($sarg,1);
                    }
                }
                $verb = VERB2();
                prt("Verbosity = $verbosity\n") if ($verb);
            } elsif ($sarg =~ /^l/) {
                if ($sarg =~ /^ll/) {
                    $load_log = 2;
                } else {
                    $load_log = 1;
                }
                prt("Set to load log at end. ($load_log)\n") if ($verb);
            } elsif ($sarg =~ /^o/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                $out_file = $sarg;
                prt("Set out file to [$out_file].\n") if ($verb);
            } else {
                pgm_exit(1,"ERROR: Invalid argument [$arg]! Try -?\n");
            }
        } else {
            $in_file = $arg;
            prt("Set input to [$in_file]\n") if ($verb);
        }
        shift @av;
    }

    if ($debug_on) {
        #prtw("WARNING: DEBUG is ON!\n");
        if (length($in_file) ==  0) {
            $in_file = $def_file;
            prt("Set DEFAULT input to [$in_file]\n");
            $in_file2 = $def_file2;
            $in_file3 = $def_file3;
        }
    }
    if (length($in_file) ==  0) {
        pgm_exit(1,"ERROR: No input files found in command!\n");
    }
    if (! -f $in_file) {
        pgm_exit(1,"ERROR: Unable to find in file [$in_file]! Check name, location...\n");
    }
}

sub give_help {
    prt("$pgmname: version $VERS\n");
    prt("Usage: $pgmname [options] in-file\n");
    prt("Options:\n");
    prt(" --help  (-h or -?) = This help, and exit 0.\n");
    prt(" --verb[n]     (-v) = Bump [or set] verbosity. def=$verbosity\n");
    prt(" --load        (-l) = Load LOG at end. ($outfile)\n");
    prt(" --out <file>  (-o) = Write output to this file.\n");
}

# eof - template.pl
