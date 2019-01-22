class I386ElfGrub < Formula
  desc "GNU GRUB 2 for i386-elf"
  homepage "https://www.gnu.org/software/grub/"
  url "https://ftp.gnu.org/gnu/grub/grub-2.02.tar.xz"
  sha256 "810b3798d316394f94096ec2797909dbf23c858e48f7b3830826b8daa06b7b0f"

  depends_on "i386-elf-gcc"

  def install
    mkdir "build" do
        system "../configure", "--target=i386-elf",
                               "--prefix=#{prefix}",
                               "--program-prefix=i386-elf-",
                               "--disable-werror",
                               "TARGET_CC=i386-elf-gcc",
                               "TARGET_OBJCOPY=i386-elf-objcopy",
                               "TARGET_STRIP=i386-elf-strip",
                               "TARGET_NM=i386-elf-nm",
                               "TARGET_RANLIB=i386-elf-ranlib"
      system "make"
      system "make", "install"
    end
  end

  test do
    assert_match "i386-elf-grub-glue-efi (GRUB) 2.02", shell_output("#{Formula["i386-elf-grub"].bin}/i386-elf-grub-glue-efi --version")
  end
end
