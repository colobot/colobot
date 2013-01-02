(defun colobot-mode ()
  "CBot mode"
  (interactive)
  (c-mode)
  (delete-trailing-whitespace)
  (indent-region (point-min) (point-max) nil)
  (untabify (point-min) (point-max)))

(add-to-list 'auto-mode-alist '("^/tmp/colobot_edit_" . colobot-mode))
(add-to-list 'auto-mode-alist '("\\.cbot$" . colobot-mode))
