<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxutil_genpw&nbsp;-- password generation</p>

<h1>Syntax</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxutil/libvxutil.h&gt;<br />
<br />
<b>void</b> vxutil_genpw(<b>char *</b><i>password</i>, <b>int</b> <i>len</i>, <b>unsigned int</b> <i>flags</i>);<br />
<br />
Link with <i>-lvxutil</i>.</code></p>

<h1>Description</h1>

<p class="block"><code>vxutil_genpw()</code> will generate a random password
according to the parameters in the <i>flags</i> argument. <i>len</i> should
denote the size of the buffer pointed to by <i>password</i>, so actually
<i>len-1</i> characters are produced.

<h2>Flags for characteristics:</h2>

<table border="1">
	<tr>
		<td class="t1n"><code><b>GENPW_1DIGIT</b></code></td>
		<td class="t1">Make sure there is at least one digit in the
		output.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>GENPW_O1DIGIT</b></code></td>
		<td class="t2">Allow digits to appear in the output password.
		If this flag is not present, no digits will ever be in the
		password.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>GENPW_1CASE</b></code></td>
		<td class="t1">Make sure there is at least one uppercase
		character in the output.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>GENPW_O1CASE</b></code></td>
		<td class="t2">Allow uppercase characters to appear in the
		output. If this flag is not present, no uppercase characters
		will ever be in the password.</td>
	</tr>
</table>

<h2>Flags for algorithms:</h2>

<p class="block">Only one of the flags can be chosen at a time.</p>

<table border="1">
	<tr>
		<td class="t1n"><code><b>GENPW_RAND</b></code></td>
		<td class="t1">Generate a truly random password. This is the
		default.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>GENPW_JP</b></code></td>
		<td class="t2">Generate a password from randomly chosen JP
		table (Hepburn romanization) indexes. See below for
		details.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>GENPW_ZH</b></code></td>
		<td class="t1">Generate a password from randomly chosen ZH
		table (Pinyin romanization) indexes. See below for
		details.</td>
	</tr>
</table>

<h1>Phonetic passwords</h1>

<p class="block">Because truly random passwords are harder to remember,
Vitalnix comes with alternate algorithms that produce so-called phonetic
passwords, that is, passwords that are random yet easily pronouncible (in
Western culture at least) and memorizable.</p>

<h2>ZH table</h2>

<p class="block"><code>GENPW_ZH</code> will choose an algorithm which randomly
picks strings from a reduced table of simplified Pinyin romanization strings of
Chinese hanzi. Similarly sounding pairs have been squashed to minimize decision
problems when a person is recollecting the password from phonetic memory. The
ZH table is quite versatile with 381 strings, so if you think the passwords are
too complex (run <a href="vxrandpw.1.php">vxrandpw</a>(1) to put you to a
test), try going with the JP table instead.</p>

<h2>JP table</h2>

<p class="block"><code>GENPW_JP</code> will choose an algorithm which randomly
picks strings from a table of Hepburn romanization strings of the Japanese base
kana plus voiced variants (e.g. "HA", "BA" and "PA", but no combinations like
"BYA"). The JP table consists of only 69 strings, so provides a pool of easy
syllables. A few minor tweaks are in place to avoid corner cases such as the
same string twice in a row, or two consecutive consonants (e.g. problematic
"NRU").</p>

<h1>Example</h1>

<p class="code"><code>
char buf[9];<br />
vxutil_genpw(buf, sizeof(buf), GENPW_JP | GENPW_1DIGIT);</code></p>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxrandpw.1.php">vxrandpw</a>(1)</p>

<?php include_once("Base-footer.php"); ?>
