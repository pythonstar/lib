# ====================================================================
# Copyright (c) 2008 Klaus Rechert All rights reserved.
#           (c) 2009 A.Kleine
# This program is free software; you can redistribute it and/or modify
# it under the same terms as Perl itself.
# ====================================================================

# $Author: akleine $
# $Id: Blur.pm,v 1.3 2009/01/17 12:29:36 akleine Exp $

package SWF::Blur;
use SWF ();

$SWF::Blur::VERSION = $SWF::VERSION;

1;

__END__

=head1 NAME

SWF::Blur - SWF blur class

=head1 SYNOPSIS

	use SWF::Blur;
	$blur = new SWF::Blur(15, 15, 2);
	$filter = SWF::Filter::newDropShadowFilter( \@colors, $blur, $shadow, 0);

=head1 DESCRIPTION

SWF::Blur is a helper class useful for SWF::Filter.

=head1 METHODS

=over

=item new SWF::Blur($blurX, $blurY, $passes)

Creates a Blur object. The object is controlled by the horizontal blur amount -blurX-, 
the vertical blur amount -blurY- and the count of passes (use 1, 2 or 3).
Use the created object as parameter for construction of some of the SWF::Filter objects.

=back

=head1 AUTHOR

	developers of 
	ming.sourceforge.net

=head1 SEE ALSO

SWF, SWF::Filter, SWF::Blur, SWF::DisplayItem

=cut
