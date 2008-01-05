<?php include_once("Base-header.php"); ?>

<h1>Intro</h1>

<p class="block"></p>

<h1>Installation - CUPS (Common Unix Printing System)</h1>

<p class="block">Unlike other print accounting solutions, the Vitalnix lpacct
hook is implemented as a CUPS filter rather than as a CUPS backend. Because
CUPS lacks the feature to dynamically define filters (such as in a text file)
as of this writing, a patch to the CUPS source is necessary, and this will most
likely continue even after pre-filters are supported in CUPS. However, all of
this has a number of advantages over backend-based accounting solutions:</p>

<ol>

<li>Backend accounting receives printer-specific language data. Data needs to
translated back into a simple raster format, or is at least interpreted right
away, i.e. directly count ink coverage. Either way, you need a decoder for
this. In the worst case, there is no decoder written yet, and in the other
case, the decoder must be kept updated/in sync with the encoder.</li>

<li>Most likely you will get an intermediate Postscript file in the filtering
process as a result of mime conversions towards printer language. For example,
if you print an image from the command line using <code>lpr foo.png</code>, the
filter chain will be <i>image/png</i>&nbsp;-&gt;
<i>application/vnd.cups-postscript</i>&nbsp;-&gt; <i>printer/*</i>. Since
Vitalnix lpacct operates on Postscript, the lpacct filter can just be inserted
after the PNG-to-PS conversion has been done.<br />

(It is probably not as fast as backend counting, but a lot less complex.)</li>

</ol>

<p class="block">So you have to get CUPS's source tarball, extract, patch and
compile it. It is advised to use the distributions packaging methods, e.g.
.src.rpm for openSUSE/SLE* or Fedora/Redhat, to keep installation and upgrade
relatively easy. Precompiled CUPS packages for openSUSE can be found in</p>

<p><a href="ftp://ftp5.gwdg.de/pub/linux/misc/suser-jengelh/SUSE-10.3/"
target="_blank"><code>ftp://ftp5.gwdg.de/pub/linux/misc/suser-jengelh/SUSE-10.3/</code></a></p>

<h1>Installation - Syslog</h1>

<p class="block">Usually there is no installation required. However, you will
need your own tools to extract it from syslog. In its simplest form, lpacct
will make a syslog entry for each print job (in fact, every Vitalnix filter
invocation) and another entry, if it was successfully sent. You can then grep
them from <code>/var/log/messages</code> and process them any way you like.
Beware of automatic system log rotation utilities.</p>

<h1>Installation - MySQL</h1>

<p class="block">You will have to create a new database for Vitalnix
accounting. The required layout of the tables is given in the
<code>lpacct_init.sql</code> file in the <code>share/</code> directory. You may
freely choose the database, but the table name is hardcoded in a lot of places,
as are (even more) the column names. Only one table will ever be used, so you
may put the required <code>printlog</code> table into an already existing
database.</p>

<h1>lpacview</h1>

<p class="block"><i>lpacview</i> is a web frontend PHP program for MySQL-stored
print accounting information that allows users to view their print jobs.
The administrator will have a complete overview of all users and is permitted
to delete entries.</p>

<h2>Cleaning database</h2>

<p class="block">As each user prints a document, details about it will be
logged into a mysql table. The table will grow until entries are manually
deleted by the administrator, usually when the accounting period is over
and it is time for cashing up. As you will see, checkboxes for convenient
deletion (per-job, per-user, all users) are provided.</p>

<h2>Cost preview</h2>

<p class="block">Because accounting and analyzing is about the same job, the
lpacct program has a limited command-line debug interface with which the cost
of a print job can be analyzed before it is actually sent to the printer.</p>

<p class="block">Since the filter program is usually called by CUPS, only
Postscript is supported. An extra flag allows for PPM/PGMs to be passed, but
that is really only for debugging and optimizing and tuning the analyzer
algorithms.</p>

<p class="block">Note that the default analysis precision (dots per inch) is
hard-coded and I do not know of a way to dynamically change it on a per-job
basis from within CUPS. With the CLI, the <code>-d</code> option may be used,
but the results may differ slightly between different DPI values. 300 DPI is
the current hardcoded default and is a good compromise between precision and
speed, slightly biased towards precision.</p>

<h2>Accounting summary</h2>

<p class="block"><i>lpacview.php</i> is a PHP script providing a web interface
that allows a user to see what print jobs were sent to the queue and what their
cost in the unit "i*A4" (intensity x ISO A4) is. Total page count is also
displayed. Detailed stats about exact CMYK ink usage can be selected.</p>

<h2>Totals overview</h2>

<p class="block"></p>

<h1>Mode of operation</h1>

<p class="block">vxlpacct, in its current form, is a non-enforcing system,
which means it does not stop a job from actually being printed. With respect to
this, vxlpacct was designed as being a post pay system.</p>

<?php include_once("Base-footer.php"); ?>
