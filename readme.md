```
we clear Dirty bit (D bit) in the PTE, which will
correspond to the page with the system structures.
During any writing access to this page, CPU will
raise a #PF (dirty-bit fault).
```


this rep try to verify this idea