# project/Makefile
SRCDIR=src
TESTDIR=test
OBJDIR=obj

all:
	@mkdir -p $(OBJDIR)
	@make -C $(SRCDIR)
	@make -C $(TESTDIR)

test:
	@make -C $(TESTDIR)
	@./$(TESTDIR)/test_main

.PHONY: clean

clean:
	@make -C $(SRCDIR) clean
	@make -C $(TESTDIR) clean
	@rm -rf $(OBJDIR)