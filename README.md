# tacc_logs
A Linux kernel log analysis tool based on a rationalized printk  

To label log entries with jobid use

echo JOBID > /sys/module/rational_printk/parameters/JOBID

before a job begins.  Note

echo 0 > /sys/module/rational_printk/parameters/JOBID

upon job completion is then necessary.



To direct rsyslog data to a particular server on the compute nodes:

Add the following lines to /etc/rsyslog.conf

# Include all config files in /etc/rsyslog.d/
$IncludeConfig /etc/rsyslog.d/*.conf

and a conf file in /etc/rsyslog.d/ with the content (for example)

if $msg contains 'RPKJOBID' then @tacc-stats.tacc.utexas.edu
&~

and the server:

Add the following lines to /etc/rsyslog.conf

# Include all config files in /etc/rsyslog.d/
$IncludeConfig /etc/rsyslog.d/*.conf

with the following conf file in /etc/rsyslog.d/

$template RemoteHost,"/var/rpk/stampede/syslog/%$YEAR%/%$MONTH%/%$DAY%/%HOSTNAME%.log"
$Ruleset remote
*.* ?RemoteHost
#if $msg contains 'RPKJOBID' then /var/log/rpk.log
#if $msg contains 'JobID' then /var/log/rpk.log
$Ruleset RSYSLOG_DefaultRuleset

# provides UDP syslog reception
$ModLoad imudp
$InputUDPServerBindRuleset remote
$UDPServerRun 514

# LICENSE
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
