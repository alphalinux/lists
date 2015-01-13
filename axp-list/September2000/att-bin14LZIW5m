#!/usr/bin/perl -w
# -------------------------------------------------------------
# -- headless.pl                                             --
# -------------------------------------------------------------
# Copyright: DCG Computers, Inc./Atipa Linux Solutions, May 2K
#
# Author: Kevin Wood
#         Concept by Anthony Lambiris
#
# Change-Log:
#     when  who  what
#  -------  ---  ----------------------------------------------
#    2000
#   13 may  klw   V.0.1 - initial version of procedure
#   16 may  amc   created service()  and file_read() subroutines
#

use strict;

my ( @_lines, @_words, $_dbg, $_size, $_INITW, $_ndx, $_securetty, $_inittab );
$_ndx       = 0;
$_dbg       = 1;
$_size      = 0;
$_inittab   = '/etc/inittab';
$_securetty = '/etc/securetty';

# -------------------------------------------------------------
# -- subroutine - file_read( message, file-name )            --
# -------------------------------------------------------------
# print message and read <file-name> into lines array
#
# returns:
#   global vars lines array and size
#
sub file_read
  {
  my ( $_file_exists, $_file_name );
  ( $_file_name ) = @_;
  $_file_exists = 0;
  if ( -e $_file_name )
     {
     my $_INIT;
     open( _INIT, $_file_name );
     @_lines = <_INIT>;
     $_size = @_lines;
     close( _INIT );
     $_file_exists = 1;
     }
   else
     {
     print "! $_file_name not found.\n";
     }
  return ( $_file_exists );
  }


sub order
   {
   my ( $_operation, $_service, $_rtn, $_pos );
   ( $_service, $_operation ) = @_;
   $_rtn = 0;
   $_pos = 0;
   if ( file_read( "/etc/rc.d/init.d/$_service" ) )
      {
      for ( $_ndx = 0; $_ndx < @_lines; $_ndx++ )
         {
         chomp $_lines[$_ndx];
         @_words = split( /[\t ]+/, $_lines[$_ndx] );
         if ( @_words < 4 )
            {
            next;
            }
         if ( $_words[1] eq 'chkconfig:' )
            {
            $_pos = 4;
            if ( $_operation eq 'start' )
               {
               $_pos = 3;
               }
            last;
            }
         }
      }
   $_rtn = $_words[$_pos];
   }

# -------------------------------------------------------------
# function to turn a service in a runlevel on or off
# -------------------------------------------------------------
sub service
   {
   my ( $_runlevel, $_order, $_service, $_state, $_initial_state, $_final_state, $_start_order, $_stop_order, $_cmd, $_cmdalt );
   ( $_runlevel, $_order, $_service, $_state ) = @_;
   $_start_order = order( $_service, 'start' );
   $_stop_order  = order( $_service, 'stop' );
   $_initial_state = 'K';
   $_final_state   = 'S';
   $_cmd = "on";
   $_cmdalt = "off";
   if ( $_dbg )
      {
      print "$_service start $_start_order,  stop $_stop_order\n";
      }
   if ( $_state eq 'off' )
      {
      $_initial_state = 'S';
      $_final_state   = 'K';
      $_cmd = "off";
      $_cmdalt = "on";
      }

   if ( $_start_order eq '0' || $_stop_order eq '0' )
      {
      print "$_order$_service is not installed.\n";
      }
   else
      {
      if ( -e "/etc/rc.d/rc$_runlevel.d/$_initial_state$_order$_service" )
         {
         print "Turning $_service $_cmd\n";
         `chkconfig --level 3 $_service $_cmd`;
         }
      else
         {
         if ( -e "/etc/rc.d/rc$_runlevel.d/$_final_state$_order$_service" )
            {
            print "$_service is already $_cmdalt\n";
            }
         }
      }
  }  #  end sub service( runlevel, order, service, 'on|off' )

# -------------------------------------------------------------
# -- subroutine - file_scan( pattern, array of lines )       --
# -------------------------------------------------------------
# Description:
#   check each entry in the lines array for pattern
#
# returns:
#   true if pattern is found, otherwise false
#
sub file_scan
   {
   my $_pattern_present = 0;
   my ( $_file_name, $_pattern );
   ( $_file_name, $_pattern ) = @_;
   if ( file_read( $_file_name ) )
      {
      for ( $_ndx = 0; $_ndx < @_lines; $_ndx++ )
         {
         chomp $_lines[$_ndx];
         if ( $_lines[$_ndx] eq $_pattern )
            {
            $_pattern_present = 1;
            print "-- $_file_name already contains '$_pattern'\n";
            last;
            }
         }
      }
   else
      {
      $_pattern_present = -1;
      }
   $_pattern_present;
   }

sub file_save
   {
   my ( $_file_name, $_saved_as );
   ( $_file_name, $_saved_as ) = @_;
   if ( -e $_saved_as )
      {
      print "-- $_saved_as already saved\n";
      }
   else
      {
      rename $_file_name, "$_saved_as";
      print "-- file $_file_name saved as $_saved_as\n";
      }
   }

sub  headless_securetty
  {
  my $_file_name;
  ( $_file_name ) = @_;
  if ( file_scan( $_file_name, "ttyS0" ) eq 0 )
     {
     file_save( $_file_name, "$_file_name.headed" );
     print "-- Adding ttyS0 to $_file_name......\n";
     open( _INITW, "> $_file_name" );
     for ( $_ndx = 0; $_ndx < $_size; $_ndx++ )
        {
        if ( $_lines[$_ndx] eq 'tty1' )
           {
           print _INITW "ttyS0\ntty1\n";
           }
        else
           {
           print _INITW "$_lines[$_ndx]\n";
           }
        }
     close( _INITW );
     }
  }


sub  headless_inittab
  {
  my $_file_name;
  ( $_file_name ) = @_;
  if ( file_scan( $_file_name,"0:2345:respawn:/sbin/mingetty ttyS0" ) eq 0 )
     {
     file_save( $_file_name, "$_file_name.headed" );
     open( _INITW,"> $_file_name" );
     for ( $_ndx = 0; $_ndx < $_size; $_ndx++ )
        {
        if ( $_lines[$_ndx] eq '1:2345:respawn:/sbin/mingetty tty1' )
           {
           print _INITW "0:2345:respawn:/sbin/mingetty ttyS0\n";
           }
        print _INITW "$_lines[$_ndx]\n";
        }
     close( _INITW );
     }
  }

# -------------------------------------------------------------
# -- subroutine - headless_make()                            --
# -------------------------------------------------------------
# Description:
#   allow machine to run headless by making changes to :
#      kudzu, keytable and gpm services
#      /etc/inittab and
#      /etc/securetty
#
# returns:
#   true if pattern is found, otherwise false
#
sub headless_make
   {
   # -------------------------------------------------------------
   #  Turn off keytable, kudzu and gpm
   # -------------------------------------------------------------
   service( '3', '85', 'gpm',      'off' );
   service( '3', '75', 'keytable', 'off' );
   service( '3', '05', 'kudzu',    'off' );

   # add ttyS0 to /etc/securetty
   headless_securetty( '/etc/securetty' );
   
   #  Add 0:2345:respawn:/sbin/mingetty ttyS0 to inittab
   headless_inittab( '/etc/inittab' );
   
   print "-- This machine is now ready to run headless.\n";
   }  #  end headless_make


sub  file_restore
  {
  my ( $_src_file, $_dst_file );
  ( $_src_file, $_dst_file ) = @_;
  if ( -e $_src_file )
     {
     print "-- moving $_src_file to $_dst_file\n";
     rename  $_src_file, $_dst_file;
     }
  else
     {
     print "-- saved file $_src_file not found, nothing to do\n";
     }
  }

sub  headless_undo
  {
  file_restore( "/etc/inittab.headed",   "/etc/inittab" );
  file_restore( "/etc/securetty.headed", "/etc/securetty" );
  service( '3',  '05',  'kudzu',    'on' );
  service( '3',  '75',  'keytable', 'on' );
  service( '3',  '85',  'gpm',      'on' );
  }

sub  headless_ask
  {
  print "-- Would you like to make this system headless?         (y) \n";
  print "--                to undo a currently headless system ? (u) \n";
  print "--                or do nothing                         (n) ";
  my $_answer;
  $_answer=<stdin>;
  chomp $_answer;
  if ( $_answer eq "y" && $_answer eq "Y" )
     {
     headless_make;
     }
  if ( $_answer eq "u" && $_answer eq "U" )
     {
     headless_undo;
     }
  exit 0;
  }
  

sub  help_show
  {
  print "\n\n./headless -[h|u|y]\n";
  print "     w/o args - changes files to make a system headless\n";
  print "                after asking for confirmation\n";
  print "     -h  - shows this info\n";
  print "     -u  - will undo changes made by a previous run of headless\n";
  print "     -y  - will make changes without interactive confirmation\n\n\n";
  }

# -------------------------------------------------------------
# verify the action to be taken
# -------------------------------------------------------------
my $argc;
$argc = @ARGV;
if ( $argc > 0 )
   {
   if ( $ARGV[0] eq '-h' )
      {
      help_show;
      }
   if ( $ARGV[0] eq '-u' )
      {
      headless_undo;
      }
   if ( $ARGV[0] eq '-y' )
      {
      headless_make;
      }
   }
else
   {
   headless_ask;
   }
