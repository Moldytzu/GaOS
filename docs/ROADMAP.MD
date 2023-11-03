# Proposed roadmap to follow in Gallium Operating System development

* Hello World []
  * Load the kernel in the higher-half using Limine []
  * Have a way to display things on the screen []
    * Serial []
    * Framebuffer with font []
* Symetric multiprocessing []
  * Enumerate MADT for other processors []
  * Implement helper functions to handle I/O APIC and Local APIC []
  * Boot-up Application Processors []
  * Jump the Application Processors in the kernel []
  * Implement spinlocks []
* Memory managemnet []
  * Implement a page frame allocator using bitmaps []
    * Make it return only higher half addresses
  * Implement a block allocator using free lists []
    * Make it return only virtual half addresses
  * Implement a virtual memory manager to handle page tables []
* Scheduling []
