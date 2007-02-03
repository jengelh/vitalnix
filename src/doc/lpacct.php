<?php include_once("Base-header.php"); ?>

<h1>Intro</h1>

<p class="block"></p>

<h1>Installation</h1>

<h2>CUPS (Common Unix Printing System)</h2>

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

<p><a href="ftp://ftp-1.gwdg.de/pub/linux/misc/suser-jengelh/SUSE-10.2/"
target="_blank"><code>ftp://ftp-1.gwdg.de/pub/linux/misc/suser-jengelh/SUSE-10.2/</code></a></p>

<h2>Syslog</h2>

<p class="block">In its simplest form, lpacct will make a syslog entry for each
print job. You can then grep them from <code>/var/log/messages</code> and
process them any way you like. Beware of automatic system log rotation
utilities.</p>

<h2>MySQL</h2>

<p class="block"></p>

<h1>Usage</h1>

<h2>Cleaning database</h2>

<p class="block">As each user prints a document, details about it will be
logged into a mysql table. The table will grow until it is manually flushed by
the administrator, usually when the accounting period is over and it is time
for cashing up. The exact details on how the collected data is used or billed
is up to the administrator or authoritative people, respectively, which is why
there are no tools besides the simple <i>lpaview</i>.</p>

<p class="block">The simplest approach to empty the table and hence reset the
counters it to do it with the following mysql statement:</p>

<p class="code"><code>truncate vxlpacct.printlog;</code></p>

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

<p class="block"><i>lpaview</i> is a CGI binary that allows a user to see what
prints job were sent to the queue and what their cost in the unit "i*A4"
(intensity x ISO A4) is. Total page count is also displayed. Detailed stats
about exact CMYK ink usage can be selected at the authentication dialog.</p>

<h2>Totals overview</h2>

<p class="block"></p>

<?php include_once("Base-footer.php"); ?>
