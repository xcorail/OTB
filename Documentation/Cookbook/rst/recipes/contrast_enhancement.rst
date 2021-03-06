Enhance local contrast
======================

Principles
~~~~~~~~~~

Sensor images have often a wide dynamic range. Whereas it is helpful to have
high precision to do complex processing, it is pretty hard to display high
dynamic images, even on modern screen as the dynamic range for basic screen is
of 8 bits while images can be encoded on 12 or 16 bits (or even more!).

.. _Figure1:
      
|image5| |image6|

The *ContrastEnhancement* application aims to reduce the image dynamic by
reorganizing it in a smarter way than just linear compression and improve the
local contrast and enhance the definitions of edges.

.. _Figure2:

|image1| |image2|

The equalization of histogram creates a look up table in order to maximize the
dynamic. The target histogram is perfectly flat.  The gain applied on each pixel
comes from the computation of the transfer function :math:`T` such that :

.. math:: \forall i \quad \int_{min}^{i*T(i)}h_{istogram}(j)dj =
          \int_{min}^{i}h_{target}(j)dj

where :math:`h_{target}` is the corresponding flat histogram with the constraint
that white and black are still white and black after equalization :

.. math:: T(min) = T(max) = 1

You can apply this transformation with the *ContrastEnhancement* application:

::

    otbcli_ContrastEnhancement -in input_image.tif
                               -out output_image.tif
                               -spatial global

It allows to compress the dynamic without losing details and contrast.

Advanced parameters
~~~~~~~~~~~~~~~~~~~

The *ContrastEnhancement* provides different options to configure the contrast
enhancement method. Let us see what there are for.

First what you want to equalize. Two modes are available:

* **luminance:** on 3 bands image, the equalization will be done on a single
  band which will be a composition of the original bands. The computed gain will
  then be applied on the different bands. The classical use of this method is to
  conserve ratio between the different color, conserve the hue.
* **channel:** each bands are equalized independently.

The other option is the local equalization. You can choose a window size that
will be use to split the image in tiles and histograms will be computed over
those tiles. Gain will be interpolated between the adjacent tiles in order to
give a smooth result.

::

    otbcli_ContrastEnhancement -in input_image.tif
                               -out output_image.tif spatial.local.h 500
                               -spatial.local.w 500
                               -mode lum

The *ContrastEnhancement* application also offers a way to limit contrast by
adjusting original histogram with the **hfact** parameter. The limitation factor
represents the limit height that can have any bucket of the histogram; the
application computes the height of the flat histogram and the maximal height is
the limitation factor time this "flat height".

.. _Figure3:

|image4|

Finally, you can ignore a particular value with the **nodata** parameter, and
also set manually your minimum and maximum value. Any value out of bound will be
ignored.


.. |image1| image:: ../Art/contrast1.png
            :scale: 30%

.. |image2| image:: ../Art/contrast2.png
            :scale: 30%

.. |image4| image:: ../Art/contrast4.png

.. |image5| image:: ../Art/contrast_orig.jpg
            :scale: 30%
            :alt: original

.. |image6| image:: ../Art/contrast_result.jpg
            :scale: 30%
            :alt: result
