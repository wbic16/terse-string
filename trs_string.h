// -----------------------------------------------------------------------------------------------------------
// Terse String
// (c) 2023 Will Bickford
// MIT License
// -----------------------------------------------------------------------------------------------------------

#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <sstream>

namespace trs
{
   static constexpr size_t SUMMARY_LINE_LENGTH = 64;
   enum Break : uint8_t
   {
      STRING     = 0x00, // 1st
      LINE       = 0x10, // 2nd (Newline)
      WORD       = 0x20, // (Space)
      SCROLL     = 0x17, // 3rd
      SECTION    = 0x18, // 4th
      CHAPTER    = 0x19, // 5th
      BOOK       = 0x1A, // 6th
      VOLUME     = 0x1C, // 7th
      COLLECTION = 0x1D, // 8th
      SERIES     = 0x1E, // 9th
      SHELF      = 0x1F, // 10th
      LIBRARY    = 0x01  // 11th
   };
   static constexpr uint8_t SCROLL_BREAK     = Break::SCROLL;
   static constexpr uint8_t SECTION_BREAK    = Break::SECTION;
   static constexpr uint8_t CHAPTER_BREAK    = Break::CHAPTER;
   static constexpr uint8_t BOOK_BREAK       = Break::BOOK;
   static constexpr uint8_t VOLUME_BREAK     = Break::VOLUME;
   static constexpr uint8_t COLLECTION_BREAK = Break::COLLECTION;
   static constexpr uint8_t SERIES_BREAK     = Break::SERIES;
   static constexpr uint8_t SHELF_BREAK      = Break::SHELF;
   static constexpr uint8_t LIBRARY_BREAK    = Break::LIBRARY;

   struct Coordinate
   {
      uint16_t LibraryID{1};
      uint16_t ShelfID{1};
      uint16_t SeriesID{1};
      uint16_t CollectionID{1};
      uint16_t VolumeID{1};
      uint16_t BookID{1};
      uint16_t ChapterID{1};
      uint16_t SectionID{1};
      uint16_t ScrollID{1};

      void libraryBreak()
      {
         LibraryID += 1;
         ShelfID = 1;
         SeriesID = 1;
         CollectionID = 1;
         VolumeID = 1;
         BookID = 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void shelfBreak()
      {
         ShelfID += 1;
         SeriesID = 1;
         CollectionID = 1;
         VolumeID = 1;
         BookID = 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void seriesBreak()
      {
         SeriesID += 1;
         CollectionID = 1;
         VolumeID = 1;
         BookID = 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void collectionBreak()
      {
         CollectionID += 1;
         VolumeID = 1;
         BookID = 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void volumeBreak()
      {
         VolumeID += 1;
         BookID = 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void bookBreak()
      {
         BookID += 1;
         ChapterID = 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void chapterBreak()
      {
         ChapterID += 1;
         SectionID = 1;
         ScrollID = 1;
      }

      void sectionBreak()
      {
         SectionID += 1;
         ScrollID = 1;
      }

      void scrollBreak()
      {
         ScrollID += 1;
      }
   };

   struct Range
   {
      Range() : start(0), end(0), type(Break::STRING) {}
      size_t start;
      size_t end;
      Break  type;
   };

   template <class T>
   struct MappedDimension
   {
      bool contains(size_t index) { return map.contains(index); }
      T& get(size_t index) {
        if (!map.contains(index)) {
          map[index] = std::make_shared<T>();
        }
        return *map[index];
      }
      std::map<size_t, std::shared_ptr<T>> map;
   };

   using Scroll = std::string;                  //  2D 0x20
   using Section = MappedDimension<Scroll>;     //  3D 0x17
   using Chapter = MappedDimension<Section>;    //  4D 0x18
   using Book = MappedDimension<Chapter>;       //  5D 0x19
   using Volume = MappedDimension<Book>;        //  6D 0x1A
   using Collection = MappedDimension<Volume>;  //  7D 0x1C
   using Series = MappedDimension<Collection>;  //  8D 0x1D
   using Shelf = MappedDimension<Series>;       //  9D 0x1E
   using Library = MappedDimension<Shelf>;      // 10D 0x1F
   using Terse = MappedDimension<Library>;      // 11D 0x01

   class string
   {
   public:
      string()
         : m_current(), m_buffer(), m_terse()
      {
      }

      string(const char* filename)
      {
         std::ifstream file;
         file.open(filename, std::ifstream::binary);
         file.seekg(0, std::ios::end);
         size_t length = file.tellg();
         file.seekg(0, std::ios::beg);
         std::vector<uint8_t> data;
         data.resize(length);
         file.read(reinterpret_cast<char*>(data.data()), length);
         if (!file)
         {
            std::cerr << "Error: expected " << length << " bytes, but read only " << file.gcount() << " bytes.\n";
            throw std::runtime_error("Failed disk read");
         }
         file.close();

         loadTerse(data);
      }

      string(const std::vector<uint8_t>& data)
         : m_current(), m_buffer(), m_terse()
      {
         loadTerse(data);
      }

      size_t size() const noexcept
      {
         size_t total = 0;
         bool atLeastOneLibrary = false;
         for (const auto& [libraryID, libPtr] : m_terse.map)
         {
            if (atLeastOneLibrary) { ++total; }
            atLeastOneLibrary = true;
            bool atLeastOneShelf = false;
            for (const auto& [shelfID, shelfPtr] : libPtr->map)
            {
               if (atLeastOneShelf) { ++total; }
               atLeastOneShelf = true;
               bool atLeastOneSeries = false;
               for (const auto& [seriesID, seriesPtr] : shelfPtr->map)
               {
                  if (atLeastOneSeries) { ++total; }
                  atLeastOneSeries = true;
                  bool atLeastOneCollection = false;
                  for (const auto& [collectionID, collectionPtr] : seriesPtr->map)
                  {
                     if (atLeastOneCollection) { ++total; }
                     atLeastOneCollection = true;
                     bool atLeastOneVolume = false;
                     for (const auto& [volumeID, volumePtr] : collectionPtr->map)
                     {
                        if (atLeastOneVolume) { ++total; }
                        atLeastOneVolume = true;
                        bool atLeastOneBook = false;
                        for (const auto& [bookID, bookPtr] : volumePtr->map)
                        {
                           if (atLeastOneBook) { ++total; }
                           atLeastOneBook = true;
                           bool atLeastOneChapter = false;
                           for (const auto& [chapterID, chapterPtr] : bookPtr->map)
                           {
                              if (atLeastOneChapter) { ++total; }
                              atLeastOneChapter = true;
                              bool atLeastOneSection = false;
                              for (const auto& [sectionID, sectionPtr] : chapterPtr->map)
                              {
                                 if (atLeastOneSection) { ++total; }
                                 atLeastOneSection = true;
                                 bool atLeastOneScroll = false;
                                 for (const auto& [scrollID, scrollPtr] : sectionPtr->map)
                                 {
                                    if (atLeastOneScroll) { ++total; }
                                    atLeastOneScroll = true;
                                    total += scrollPtr->size();
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         return total;
      }

      void expand() noexcept
      {
         std::vector<uint8_t> expanded;
         expanded.reserve(size());
         bool atLeastOneLibrary = false;
         for (const auto& [libraryID, libPtr] : m_terse.map)
         {
            if (atLeastOneLibrary) { expanded.push_back(LIBRARY_BREAK); }
            atLeastOneLibrary = true;
            bool atLeastOneShelf = false;
            for (const auto& [shelfID, shelfPtr] : libPtr->map)
            {
               if (atLeastOneShelf) { expanded.push_back(LIBRARY_BREAK); }
               atLeastOneShelf = true;
               bool atLeastOneSeries = false;
               for (const auto& [seriesID, seriesPtr] : shelfPtr->map)
               {
                  if (atLeastOneSeries) { expanded.push_back(SHELF_BREAK); }
                  atLeastOneSeries = true;
                  bool atLeastOneCollection = false;
                  for (const auto& [collectionID, collectionPtr] : seriesPtr->map)
                  {
                     if (atLeastOneCollection) { expanded.push_back(SERIES_BREAK); }
                     atLeastOneCollection = true;
                     bool atLeastOneVolume = false;
                     for (const auto& [volumeID, volumePtr] : collectionPtr->map)
                     {
                        if (atLeastOneVolume) { expanded.push_back(COLLECTION_BREAK); }
                        atLeastOneVolume = true;
                        bool atLeastOneBook = false;
                        for (const auto& [bookID, bookPtr] : volumePtr->map)
                        {
                           if (atLeastOneBook) { expanded.push_back(VOLUME_BREAK); }
                           atLeastOneBook = true;
                           bool atLeastOneChapter = false;
                           for (const auto& [chapterID, chapterPtr] : bookPtr->map)
                           {
                              if (atLeastOneChapter) { expanded.push_back(BOOK_BREAK); }
                              atLeastOneChapter = true;
                              bool atLeastOneSection = false;
                              for (const auto& [sectionID, sectionPtr] : chapterPtr->map)
                              {
                                 if (atLeastOneSection) { expanded.push_back(CHAPTER_BREAK); }
                                 atLeastOneSection = true;
                                 bool atLeastOneScroll = false;
                                 for (const auto& [scrollID, scrollPtr] : sectionPtr->map)
                                 {
                                    if (atLeastOneScroll) { expanded.push_back(SECTION_BREAK); }
                                    atLeastOneScroll = true;
                                    for (char c : *scrollPtr)
                                    {
                                       if (c == '\n')
                                       {
                                          expanded.push_back(SCROLL_BREAK);
                                       }
                                       else
                                       {
                                          expanded.push_back(c);
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         loadTerse(expanded);
      }

      void contract() noexcept
      {
         std::vector<uint8_t> contracted;
         contracted.reserve(size());
         bool atLeastOneLibrary = false;
         for (const auto& [libraryID, libPtr] : m_terse.map)
         {
            if (atLeastOneLibrary) { contracted.push_back(SHELF_BREAK); }
            atLeastOneLibrary = true;
            bool atLeastOneShelf = false;
            for (const auto& [shelfID, shelfPtr] : libPtr->map)
            {
               if (atLeastOneShelf) { contracted.push_back(SERIES_BREAK); }
               atLeastOneShelf = true;
               bool atLeastOneSeries = false;
               for (const auto& [seriesID, seriesPtr] : shelfPtr->map)
               {
                  if (atLeastOneSeries) { contracted.push_back(COLLECTION_BREAK); }
                  atLeastOneSeries = true;
                  bool atLeastOneCollection = false;
                  for (const auto& [collectionID, collectionPtr] : seriesPtr->map)
                  {
                     if (atLeastOneCollection) { contracted.push_back(VOLUME_BREAK); }
                     atLeastOneCollection = true;
                     bool atLeastOneVolume = false;
                     for (const auto& [volumeID, volumePtr] : collectionPtr->map)
                     {
                        if (atLeastOneVolume) { contracted.push_back(BOOK_BREAK); }
                        atLeastOneVolume = true;
                        bool atLeastOneBook = false;
                        for (const auto& [bookID, bookPtr] : volumePtr->map)
                        {
                           if (atLeastOneBook) { contracted.push_back(CHAPTER_BREAK); }
                           atLeastOneBook = true;
                           bool atLeastOneChapter = false;
                           for (const auto& [chapterID, chapterPtr] : bookPtr->map)
                           {
                              if (atLeastOneChapter) { contracted.push_back(SECTION_BREAK); }
                              atLeastOneChapter = true;
                              bool atLeastOneSection = false;
                              for (const auto& [sectionID, sectionPtr] : chapterPtr->map)
                              {
                                 if (atLeastOneSection) { contracted.push_back(SCROLL_BREAK); }
                                 atLeastOneSection = true;
                                 bool atLeastOneScroll = false;
                                 for (const auto& [scrollID, scrollPtr] : sectionPtr->map)
                                 {
                                    if (atLeastOneScroll) { contracted.push_back('\n'); }
                                    atLeastOneScroll = true;
                                    for (char c : *scrollPtr)
                                    {
                                       contracted.push_back(c);
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         loadTerse(contracted);
      }

      std::string getNavTree() const
      {
         using namespace std;
         stringstream output;
         for (const auto& [libraryID, libPtr] : m_terse.map)
         {
            output << " * Library #" << libraryID << endl;
            for (const auto& [shelfID, shelfPtr] : libPtr->map)
            {
               output << "   * Shelf #" << shelfID << endl;
               for (const auto& [seriesID, seriesPtr] : shelfPtr->map)
               {
                  output << "     * Series #" << seriesID << endl;
                  for (const auto& [collectionID, collectionPtr] : seriesPtr->map)
                  {
                     output << "       * Collection #" << collectionID << endl;
                     for (const auto& [volumeID, volumePtr] : collectionPtr->map)
                     {
                        output << "         * Volume #" << volumeID << endl;
                        for (const auto& [bookID, bookPtr] : volumePtr->map)
                        {
                           output << "           * Book #" << bookID << endl;
                           for (const auto& [chapterID, chapterPtr] : bookPtr->map)
                           {
                              output << "             * Chapter #" << chapterID << endl;
                              for (const auto& [sectionID, sectionPtr] : chapterPtr->map)
                              {
                                 output << "               * Section #" << sectionID << endl;
                                 for (const auto& [scrollID, scrollPtr] : sectionPtr->map)
                                 {
                                    output << "                 * Scroll #" << scrollID << ": ";
                                    output << getSummaryText(*scrollPtr);
                                    output << endl;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
         return output.str();
      }

      static std::string getSummaryText(const std::string& text)
      {
         const size_t length = text.length() > SUMMARY_LINE_LENGTH ? static_cast<size_t>(SUMMARY_LINE_LENGTH) : text.length();
         std::string attempt = text.substr(0, length);
         auto iter = std::find(attempt.begin(), attempt.end(), '\n');
         if (iter == std::end(attempt))
         {
            return attempt;
         }
         return std::string(attempt.begin(), iter);
      }

      void save(const char* filename) const
      {
         const std::string text = toString();
         std::ofstream output;
         output.open(filename, std::ofstream::out | std::ofstream::binary);
         output << text;
         output.close();
      }

      void loadTerse(const std::vector<uint8_t>& data)
      {
         m_terse = {};
         m_buffer.clear();
         m_current.start = 0;
         for (const uint8_t ith : data)
         {
            switch (ith)
            {
               case LIBRARY_BREAK:
                  setTerseText();
                  m_coords.libraryBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case SHELF_BREAK:
                  setTerseText();
                  m_coords.shelfBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case SERIES_BREAK:
                  setTerseText();
                  m_coords.seriesBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case COLLECTION_BREAK:
                  setTerseText();
                  m_coords.collectionBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case VOLUME_BREAK:
                  setTerseText();
                  m_coords.volumeBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case BOOK_BREAK:
                  setTerseText();
                  m_coords.bookBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case CHAPTER_BREAK:
                  setTerseText();
                  m_coords.chapterBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case SECTION_BREAK:
                  setTerseText();
                  m_coords.sectionBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               case SCROLL_BREAK:
                  setTerseText();
                  m_coords.scrollBreak();
                  m_buffer.clear();
                  m_current.end = 0;
                  break;
               default:
                  m_buffer.push_back(ith);
                  ++m_current.end;
                  break;
            }
         }
         setTerseText();
      }

      uint64_t pageCount() const
      {
         return m_pages;
      }

      std::ostream& operator<<(std::ostream& os) const
      {
         os << toString();
         return os;
      }

      operator std::string() const
      {
         return toString();
      }

      std::string toString() const
      {
         std::stringstream output;
         bool atLeastOneLibrary = false;
         for (auto& [libraryID, libPtr] : m_terse.map)
         {
            if (atLeastOneLibrary)
            {
               output << LIBRARY_BREAK;
            }
            atLeastOneLibrary = true;
            bool atLeastOneShelf = false;
            for (auto& [shelfID, shelfPtr] : libPtr->map)
            {
               if (atLeastOneShelf)
               {
                  output << SHELF_BREAK;
               }
               atLeastOneShelf = true;
               bool atLeastOneSeries = false;
               for (auto& [seriesID, seriesPtr] : shelfPtr->map)
               {
                  if (atLeastOneSeries)
                  {
                     output << SERIES_BREAK;
                  }
                  atLeastOneSeries = true;
                  bool atLeastOneCollection = false;
                  for (auto& [collectionID, collectionPtr] : seriesPtr->map)
                  {
                     if (atLeastOneCollection)
                     {
                        output << COLLECTION_BREAK;
                     }
                     atLeastOneCollection = true;
                     bool atLeastOneVolume = false;
                     for (auto& [volumeID, volumePtr] : collectionPtr->map)
                     {
                        if (atLeastOneVolume)
                        {
                           output << VOLUME_BREAK;
                        }
                        atLeastOneVolume = true;
                        bool atLeastOneBook = false;
                        for (auto& [bookID, bookPtr] : volumePtr->map)
                        {
                           if (atLeastOneBook)
                           {
                              output << BOOK_BREAK;
                           }
                           atLeastOneBook = true;
                           bool atLeastOneChapter = false;
                           for (auto& [chapterID, chapterPtr] : bookPtr->map)
                           {
                              if (atLeastOneChapter)
                              {
                                 output << CHAPTER_BREAK;
                              }
                              atLeastOneChapter = true;
                              bool atLeastOneSection = false;
                              for (auto& [sectionID, sectionPtr] : chapterPtr->map)
                              {                                 
                                 if (atLeastOneSection)
                                 {
                                    output << SECTION_BREAK;
                                 }
                                 atLeastOneSection = true;
                                 bool atLeastOneScroll = false;
                                 for (auto& [scrollID, scrollPtr] : sectionPtr->map)
                                 {                                    
                                    if (atLeastOneScroll)
                                    {
                                       output << SCROLL_BREAK;
                                    }
                                    atLeastOneScroll = true;
                                    output << *scrollPtr;
                                 }                                         
                              }                                     
                           }                                 
                        }                            
                     }                         
                  }                     
               }                 
            }             
         }
         return output.str();
      }
   private:
      void setTerseText()
      {
         setLibraryText(m_terse.get(m_coords.LibraryID));
      }

      void setLibraryText(Library& node)
      {
         setShelfText(node.get(m_coords.ShelfID));
      }

      void setShelfText(Shelf& node)
      {
         setSeriesText(node.get(m_coords.SeriesID));
      }

      void setSeriesText(Series& node)
      {
         setCollectionText(node.get(m_coords.CollectionID));
      }

      void setCollectionText(Collection& node)
      {
         setVolumeText(node.get(m_coords.VolumeID));
      }

      void setVolumeText(Volume& node)
      {
         setBookText(node.get(m_coords.BookID));
      }

      void setBookText(Book& node)
      {
         setChapterText(node.get(m_coords.ChapterID));
      }

      void setChapterText(Chapter& node)
      {
         setSectionText(node.get(m_coords.SectionID));
      }

      void setSectionText(Section& node)
      {
         setScrollText(node.get(m_coords.ScrollID));
      }

      void setScrollText(std::string& node)
      {
         if (m_current.end < 1)
         {
            return;
         }
         node = std::string(m_buffer.cbegin(), m_buffer.cend());
         ++m_pages;
      }

      Range m_current;
      Coordinate m_coords;
      std::vector<uint8_t> m_buffer; // utf8 text
      Terse m_terse;
      uint64_t m_pages{ 0 };
   };
}












