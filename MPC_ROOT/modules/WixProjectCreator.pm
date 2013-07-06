package WixProjectCreator;

# ************************************************************
# Description   : A Wix Project Creator
# Author        : James H. Hill / Charles Calkins
# $Id$
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use ProjectCreator;
use WinProjectBase;
use XMLProjectBase;
use GUID;

use vars qw(@ISA);
@ISA = qw(XMLProjectBase WinProjectBase ProjectCreator);

# ************************************************************
# Data Section
# ************************************************************

## NOTE: We call the constant as a function to support Perl 5.6.
my %info = (Creator::cplusplus() => {'ext'      => '.wxi',
                                     'dllexe'   => 'wix',
                                     'libexe'   => 'wix',
                                     'dll'      => 'wix',
                                     'lib'      => 'wix',
                                     'template' => 'wix',
                                    },
            Creator::csharp() => {'ext'      => '.wxi',
                                  'dllexe'   => 'wixcs',
                                  'libexe'   => 'wixcs',
                                  'dll'      => 'wixcs',
                                  'lib'      => 'wixcs',
                                  'template' => 'wixcs',
                                 },
           );

# ************************************************************
# Subroutine Section
# ************************************************************

sub languageSupported {
  return defined $info{$_[0]->get_language()};
}

sub convert_all_variables {
  return 1;
}

sub requires_forward_slashes {
  return 1;
}

sub expand_variables_from_template_values {
  return 1;
}

sub warn_useless_project {
  return 0;
}

sub convert_slashes {
  return 0;
}


sub is_culture_code {
  my $culture_code = shift;

  # from http://sharpertutorials.com/list-of-culture-codes/
  my @culture_codes = (
	'af', 'hu-HU', 'af-ZA', 'is', 'sq', 'is-IS',
	'sq-AL', 'id', 'ar', 'id-ID', 'ar-DZ', 'it',
	'ar-BH', 'it-IT', 'ar-EG', 'it-CH', 'ar-IQ', 'ja',
	'ar-JO', 'ja-JP', 'ar-KW', 'kn', 'ar-LB', 'kn-IN',
	'ar-LY', 'kk', 'ar-MA', 'kk-KZ', 'ar-OM', 'kok',
	'ar-QA', 'kok-IN', 'ar-SA', 'ko', 'ar-SY', 'ko-KR',
	'ar-TN', 'ky', 'ar-AE', 'ky-KG', 'ar-YE', 'lv',
	'hy', 'lv-LV', 'hy-AM', 'lt', 'az', 'lt-LT',
	'az-AZ-Cyrl', 'mk', 'az-AZ-Latn', 'mk-MK', 'eu', 'ms',
	'eu-ES', 'ms-BN', 'be', 'ms-MY', 'be-BY', 'mr',
	'bg', 'mr-IN', 'bg-BG', 'mn', 'ca', 'mn-MN',
	'ca-ES', 'no', 'zh-HK', 'nb-NO', 'zh-MO', 'nn-NO',
	'zh-CN', 'pl', 'zh-CHS', 'pl-PL', 'zh-SG', 'pt',
	'zh-TW', 'pt-BR', 'zh-CHT', 'pt-PT', 'hr', 'pa',
	'hr-HR', 'pa-IN', 'cs', 'ro', 'cs-CZ', 'ro-RO',
	'da', 'ru', 'da-DK', 'ru-RU', 'div', 'sa',
	'div-MV', 'sa-IN', 'nl', 'sr-SP-Cyrl', 'nl-BE', 'sr-SP-Latn',
	'nl-NL', 'sk', 'en', 'sk-SK', 'en-AU', 'sl',
	'en-BZ', 'sl-SI', 'en-CA', 'es', 'en-CB', 'es-AR',
	'en-IE', 'es-BO', 'en-JM', 'es-CL', 'en-NZ', 'es-CO',
	'en-PH', 'es-CR', 'en-ZA', 'es-DO', 'en-TT', 'es-EC',
	'en-GB', 'es-SV', 'en-US', 'es-GT', 'en-ZW', 'es-HN',
	'et', 'es-MX', 'et-EE', 'es-NI', 'fo', 'es-PA',
	'fo-FO', 'es-PY', 'fa', 'es-PE', 'fa-IR', 'es-PR',
	'fi', 'es-ES', 'fi-FI', 'es-UY', 'fr', 'es-VE',
	'fr-BE', 'sw', 'fr-CA', 'sw-KE', 'fr-FR', 'sv',
	'fr-LU', 'sv-FI', 'fr-MC', 'sv-SE', 'fr-CH', 'syr',
	'gl', 'syr-SY', 'gl-ES', 'ta', 'ka', 'ta-IN',
	'ka-GE', 'tt', 'de', 'tt-RU', 'de-AT', 'te',
	'de-DE', 'te-IN', 'de-LI', 'th', 'de-LU', 'th-TH',
	'de-CH', 'tr', 'el', 'tr-TR', 'el-GR', 'uk',
	'gu', 'uk-UA', 'gu-IN', 'ur', 'he', 'ur-PK',
	'he-IL', 'uz', 'hi', 'uz-UZ-Cyrl', 'hi-IN', 'uz-UZ-Latn',
	'hu', 'vi');

  return 1 if (exists {map { $_ => 1 } @culture_codes}->{$culture_code});
  return 0;
}




sub fill_value {
  my($self, $name) = @_;

  if ($name eq 'guid') {
    ## Return a repeatable GUID for use within the template.  The values
    ## provided will be hashed and returned in a format expected by Wix.
    return GUID::generate($self->project_file_name(),
                          $self->{'current_input'}, $self->getcwd());
  }
  elsif ($name eq 'source_directory') {
    my $source;

    if ($self->get_assignment('sharedname')) {
      $source = $self->get_assignment('dllout');

      if ($source eq '') {
        $source = $self->get_assignment('libout');
      }
    }
    elsif ($self->get_assignment('staticname')) {
      $source = $self->get_assignment('libout');
    }
    else {
      $source = $self->get_assignment('exeout');
    }

    ## Check for a variable in the source directory. We have to make
    ## sure we transform this correctly for WIX by adding the correct
    ## prefix. Otherwise, WIX will complain.
    if (defined $source && $source =~ /.*?\$\((.+?)\).*/) {
      my $prefix;
      my $varname = $1;

      if ($ENV{$varname}) {
        $prefix = "env";
      }
      else {
        $prefix = "var";
      }

      ## Add the correct prefix to the variable.
      $source =~ s/$varname/$prefix.$varname/g;
    }

    return $source;
  }
  elsif ($name eq 'cultures') {

    my $crlf = $self->crlf();

    # iterate over resx_files, make list of culture abbreviations
    my @resx_files  = $self->get_component_list('resx_files');

    my %cultures = ();
    foreach my $resx_file (@resx_files) {
      my @parts = split('\.', $resx_file);
      if ($parts[-1] eq 'resx') {  # if the file is a .resx file
        if (is_culture_code($parts[-2])) { # if a culture is specified
          $cultures{$parts[-2]} = 1;  # remember that culture
        }
        else {
          $cultures{'_neutral_'} = 1;  # have a neutral culture
        }
      }
    }

    # flatten into a string
    my $found_cultures = '';
    foreach my $culture (keys %cultures) {
      $found_cultures = $found_cultures . $culture . ' ';
    }

    return $found_cultures;
  }
  return undef;
}


sub get_info_hash {
  my($self, $key) = @_;

  ## If we have the setting in our information map, the use it.
  return $info{$key} if (defined $info{$key});
}

sub project_file_extension {
  return $_[0]->get_info_hash($_[0]->get_language())->{'ext'};
}


sub get_dll_exe_template_input_file {
  return $_[0]->get_info_hash($_[0]->get_language())->{'dllexe'};
}


sub get_lib_exe_template_input_file {
  return $_[0]->get_info_hash($_[0]->get_language())->{'libexe'};
}


sub get_dll_template_input_file {
  return $_[0]->get_info_hash($_[0]->get_language())->{'dll'};
}


sub get_lib_template_input_file {
  return $_[0]->get_info_hash($_[0]->get_language())->{'lib'};
}


sub get_template {
  return $_[0]->get_info_hash($_[0]->get_language())->{'template'};
}


1;
