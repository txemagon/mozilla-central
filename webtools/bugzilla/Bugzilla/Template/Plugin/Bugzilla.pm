# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Bradley Baetz <bbaetz@student.usyd.edu.au>
#

package Bugzilla::Template::Plugin::Bugzilla;

use strict;

use base qw(Template::Plugin);

use Bugzilla;

sub new {
    my ($class, $context) = @_;

    return Bugzilla->instance;
}

1;

__END__

=head1 NAME

Bugzilla::Template::Plugin::Bugzilla

=head1 DESCRIPTION

Template Toolkit plugin to allow access to the persistent C<Bugzilla>
object.

=head1 SEE ALSO

L<Bugzilla>, L<Template::Plugin>

