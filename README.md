# [Krafton Jungle Week06] malloc κ΅¬ν

<br>

- π μ§ν κΈ°κ° : 2022. 11. 30. ~ 2022. 12. 07.
- π κ³Όμ  μ€λͺ : [GUIDELINES.md](./GUIDELINES.md)
- π­ κ°λ° μΌμ§ : [[Krafton Jungle | TIL_22.12.05 - 07] Malloc Lab κ΅¬ν](https://olive-su.tistory.com/428)
- π ν¨μ μ€λͺ κ΄λ ¨ λνλ¨ΌνΈ : [Jungle olive-su | malloc-Lab](https://jungle-olivesu.netlify.app/malloc-lab/html/mm_8c.html) Β© doxygen
- π μ°Έκ³  μλ£ : [cmu_malloclab](./malloclab.pdf)

<br>

---

### π TEST RESULT

- implicit, next-fit, improved realloc

<br>

- input

```bash
>>> make
```

- option
  - `-V` : μμΈ μ μ νμΈ
  - `-f` : νΉμ  νμ€νΈ μΌμ΄μ€ μ€ν

<br>

- output

```bash
>>> Results for mm malloc:
>>> trace  valid  util     ops      secs  Kops
>>>  0       yes   90%    5694  0.002171  2623
>>>  1       yes   91%    5848  0.001442  4057
>>>  2       yes   95%    6648  0.004505  1476
>>>  3       yes   97%    5380  0.004612  1166
>>>  4       yes   66%   14400  0.000159 90509
>>>  5       yes   92%    4800  0.005821   825
>>>  6       yes   90%    4800  0.005282   909
>>>  7       yes   55%   12000  0.023977   500
>>>  8       yes   51%   24000  0.010755  2231
>>>  9       yes   76%   14401  0.000260 55388
>>> 10       yes   46%   14401  0.000147 98300
>>> Total          77%  112372  0.059131  1900
>>>
>>> Perf index = 46 (util) + 40 (thru) = 86/100
```

<br>

<br>

|[<img src="https://user-images.githubusercontent.com/67156494/210132248-9240df67-183b-4f62-9a8d-3636537e83ed.png" width=120>](https://github.com/Krafton-Jungle-W06-Team07/Team07-malloc-lab)
|-----|
| [Week06 Team 7 Repository](https://github.com/Krafton-Jungle-W06-Team07/Team07-malloc-lab) |
